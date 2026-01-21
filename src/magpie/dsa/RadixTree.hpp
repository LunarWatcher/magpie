#pragma once

#include "magpie/routing/BaseRoute.hpp"
#include "magpie/routing/Compile.hpp"
#include <deque>
#include <optional>
#include <vector>
#include <memory>
#include <string_view>


namespace magpie::dsa {

enum class MatchMode {
    NO_MATCH,
    SINGLE_SEGMENT,
    // Currently unused, reserved for future {path} placeholder
    CATCHALL
};

template <typename Value>
struct Node {
    std::optional<Value> value;
    int weight;

    std::vector<std::shared_ptr<Node<Value>>> childNodes;

    virtual ~Node() = default;

    virtual MatchMode match(const std::string_view& segment) = 0;
    constexpr virtual bool operator==(const std::string_view& pathSegment) = 0;
};

template <typename Value>
struct RootNode : public Node<Value> {
    // this is unused, but needs to be overridden so we can exploit Node::otherNodes as standardised storage. Right now,
    // it's just a vector, but if a supportive 
    MatchMode match(const std::string_view&) override { return MatchMode::SINGLE_SEGMENT; }
    constexpr virtual bool operator==(const std::string_view&) override { return true; }
};

template <typename Value>
struct StandardNode : public Node<Value> {
    std::string segment;
    StandardNode(const std::string& segment) : segment(segment) {
        this->weight = 69000;
    }

    MatchMode match(const std::string_view& segment) override {
        return segment == this->segment ? MatchMode::SINGLE_SEGMENT : MatchMode::NO_MATCH;
    }
    constexpr virtual bool operator==(const std::string_view& pathSegment) override { 
        return pathSegment == this->segment;
    }
};

template <typename Value, typename NodeType>
struct TemplateNode : public Node<Value> {
    TemplateNode() {
        if constexpr (std::is_same_v<NodeType, std::string_view>) {
            this->weight = 1000;
        } else if constexpr (std::is_same_v<NodeType, int64_t>) {
            this->weight = 2000;
        } else {
            static_assert(false, "Invalid type");
        }
    }
    virtual MatchMode match(const std::string_view& segment) override {
        // Notes:
        // - We do not care about integer/double overflow here. This is a specific kind of error that we don't want to
        //   handle while matching routes
        // - No major parsing should be done here; if complex parsing is required to determine if a segment matches, it
        //   probably isn't a placeholder we want to support
        if constexpr (std::is_same_v<NodeType, std::string_view>) {
            return MatchMode::SINGLE_SEGMENT;
        } else if constexpr (std::is_same_v<NodeType, int64_t>) {
            for (size_t i = 0; i < segment.size() - 1; ++i) {
                auto c = segment.at(i);
                if ((c < '0' || c > '9') && c != '+' && c != '-') {
                    return MatchMode::NO_MATCH;
                }
            }
            return MatchMode::SINGLE_SEGMENT;
        } else {
            static_assert(false, "Invalid type");
        }
    }

    constexpr virtual bool operator==(const std::string_view& pathSegment) override { 
        if constexpr (std::is_same_v<NodeType, std::string_view>) {
            return pathSegment.starts_with("{string}");
        } else if constexpr (std::is_same_v<NodeType, int64_t>) {
            return pathSegment.starts_with("{int}");
        } else {
            static_assert(false, "Invalid type");
        }
    }
};


// This isn't a radix tree right now, but that's the plan, and that's good enough for now
template <typename Value>
class RadixTree {
private:
    std::shared_ptr<RootNode<Value>> root;

public:
    RadixTree(): root(std::make_shared<RootNode<Value>>()) {

    }

    std::optional<Value> getRoute(const std::vector<std::string_view>& route) const {
        std::shared_ptr<Node<Value>> node = root;

        for (auto& segment : route) {
            bool hasMatch = false;
            for (auto& childNode : node->childNodes) {
                auto matchType = childNode->match(segment);
                if (matchType == MatchMode::SINGLE_SEGMENT) {
                    node = childNode;
                    hasMatch = true;
                    break;
                } else if (matchType == MatchMode::CATCHALL) {
                    return childNode->value;
                }
            }

            if (!hasMatch) {
                return std::nullopt;
            }
        }
        return node->value;
    }

    constexpr void pushRoute(const Value& value, const std::vector<std::string_view>& route) {
        std::shared_ptr<Node<Value>> node = root;

        for (auto& segment : route) {
            bool hasRoute = false;
            // This search is shit, but this code is complex and I want somewhere to start
            for (auto& childNode : node->childNodes) {
                if (*childNode == segment) {
                    node = childNode;
                    hasRoute = true;
                    break;
                }
            }

            if (!hasRoute) {
                std::shared_ptr<Node<Value>> newNode;

                // TODO: this could be constexpr, but that means adding a FixedString or ConstString split method too
                if (segment.starts_with("{string}")) {
                    newNode = std::make_shared<TemplateNode<Value, routing::TypeInfo<"{string}">::type>>(); 
                } else if (segment.starts_with("{int}")) {
                    newNode = std::make_shared<TemplateNode<Value, routing::TypeInfo<"{int}">::type>>(); 
                } else {
                    if (segment.find('{') != std::string_view::npos) {
                        [[unlikely]]
                        throw std::runtime_error(
                            std::string(segment) + " is an invalid template that wasn't discarded. This should never happen"
                        );
                    }

                    newNode = std::make_shared<StandardNode<Value>>(std::string(segment));
                }

                node->childNodes.push_back(newNode);

                std::sort(
                    node->childNodes.begin(),
                    node->childNodes.end(),
                    [](const auto& a, const auto& b) {
                        return a->weight > b->weight;
                    }
                );

                node = newNode;
            }
        }
        if (node->value.has_value()) {
            throw std::runtime_error("Duplicate route declared");
        }
        node->value = value;
    }
};

}
