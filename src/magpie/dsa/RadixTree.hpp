#pragma once

#include "magpie/application/Methods.hpp"
#include "magpie/logger/Logger.hpp"
#include "magpie/routing/Compile.hpp"

#include <variant>
#include <vector>
#include <memory>
#include <string_view>


namespace magpie::dsa {

enum class MatchMode {
    NoMatch,
    SingleSegment,
    // Currently unused, reserved for future {path} placeholder
    CatchAll
};

template <typename Value>
struct Node {
    std::unordered_map<Method::HttpMethod, Value> value;

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
    MatchMode match(const std::string_view&) override { return MatchMode::SingleSegment; }
    constexpr virtual bool operator==(const std::string_view&) override { return true; }
};

template <typename Value>
struct StandardNode : public Node<Value> {
    std::string segment;
    StandardNode(const std::string& segment) : segment(segment) {
        this->weight = 69000;
    }

    MatchMode match(const std::string_view& segment) override {
        return segment == this->segment ? MatchMode::SingleSegment : MatchMode::NoMatch;
    }
    constexpr virtual bool operator==(const std::string_view& pathSegment) override { 
        return pathSegment == this->segment;
    }
};

template <typename Value, routing::FixedString Type>
struct TemplateNode : public Node<Value> {
    using TI = routing::TypeInfo<Type>;
    
    TemplateNode() {
        this->weight = TI::weight;
    }
    virtual MatchMode match(const std::string_view& segment) override {
        // Notes:
        // - We do not care about integer/double overflow here. This is a specific kind of error that we don't want to
        //   handle while matching routes
        // - No major parsing should be done here; if complex parsing is required to determine if a segment matches, it
        //   probably isn't a placeholder we want to support
        if constexpr (Type == "{string}") {
            return MatchMode::SingleSegment;
        } else if constexpr (Type == "{int}") {
            for (size_t i = 0; i < segment.size(); ++i) {
                auto c = segment.at(i);
                if ((c < '0' || c > '9') && c != '+' && c != '-') {
                    return MatchMode::NoMatch;
                }
            }
            return MatchMode::SingleSegment;
        } else if constexpr (Type == "{path}") {
            return MatchMode::CatchAll;
        } else {
            static_assert(false, "Invalid type");
        }
    }

    constexpr virtual bool operator==(const std::string_view& pathSegment) override { 
        return Type == pathSegment;
    }
};

enum class FindError {
    /**
     * Indicates that the tree failed to find a route. When returned, this means a 404 should be returned to the end
     * user.
     */
    NoMatch,
    /**
     * Indicates that the route does exist, but that the supplied HttpMethod isn't matched by any handlers. When
     * returned, this means a 405 should be returned to the end-user
     */
    IllegalMethod
};

// This isn't a radix tree right now, but that's the plan, and that's good enough for now
template <typename Value>
class RadixTree {
private:
    std::shared_ptr<RootNode<Value>> root;

public:
    RadixTree(): root(std::make_shared<RootNode<Value>>()) {

    }

    /**
     * Takes an input route and method and identifies the associated handler (the Value type). 
     *
     * \param processedSegments[out]    How many segments in the route that actually were processed. Should only be used
     *                                  to address CatchAll routes
     * \returns either a Value, meaning the underlying route type, or a FindError. See FindError for a description of
     *          its possible values.
     */
    std::variant<FindError, Value> getRoute(
        std::vector<std::string_view>& route,
        Method::HttpMethod method,
        size_t& processedSegments
    ) const {
        std::shared_ptr<Node<Value>> node = root;

        for (auto& segment : route) {
            ++processedSegments;
            bool hasMatch = false;
            for (auto& childNode : node->childNodes) {
                auto matchType = childNode->match(segment);
                if (matchType == MatchMode::SingleSegment) {
                    node = childNode;
                    hasMatch = true;
                    break;
                } else if (matchType == MatchMode::CatchAll) {
                    node = childNode;
                    hasMatch = true;

                    goto endFor;
                }
            }

            if (!hasMatch) {
                return FindError::NoMatch;
            }
        }
    endFor:
        // Necessary to avoid incorrect IllegalMethod in sub-routes
        // Given a server with the route /some/path, / is technically also defined with `value.size() == 0`, as it isn't
        // a valid route.
        // This check correctly handles those cases.
        if (node->value.size() == 0) {
            // This allows for more lenient route handling
            // First, check if we need to match a / deeper. This allows `/subpath` to match `/subpath/`
            if (route.back() != "/") {
                for (auto& child : node->childNodes) {
                    auto matchType = child->match("/");
                    if (matchType == MatchMode::SingleSegment) {
                        node = child;
                        processedSegments = 0;
                        route.push_back("/");
                        // Recurse down to allow /subpath/ to exist standalone
                        return getRoute(
                            route,
                            method,
                            processedSegments
                        );
                    }
                }
            } else {
                // next, check if there's a catchall, This allows /subpath/ to match /subpath/{path}
                // For upgrades from /subpath, this happens on the next recursion
                for (auto& child : node->childNodes) {
                    auto matchType = child->match("index.html");
                    if (matchType == MatchMode::CatchAll) {
                        node = child;
                        ++processedSegments;
                        route.push_back("");
                        break;
                    }
                }
            }
            if (node->value.size() == 0) {
                return FindError::NoMatch;
            }
        }
        auto it = node->value.find(method);
        if (it == node->value.end()) {
            return FindError::IllegalMethod;
        }
        return it->second;
    }

    constexpr void pushRoute(
        const Value& value,
        Method::HttpMethod method,
        const std::vector<std::string_view>& route
    ) {
        std::shared_ptr<Node<Value>> node = root;

        for (auto& segment : route) {
            bool hasRoute = false;
            // This search is shit, but this code is complex and I want somewhere to start
            //
            // This checks if we have an existing node that matches the segment
            for (auto& childNode : node->childNodes) {
                if (*childNode == segment) {
                    node = childNode;
                    hasRoute = true;
                    break;
                }
            }

            // If we don't have an existing node, we push a new one. This is necessary for all the intermediates to be
            // populated.
            if (!hasRoute) {
                std::shared_ptr<Node<Value>> newNode;

                // TODO: this could be constexpr, but that means adding a FixedString or ConstString split method too
                if (segment.starts_with("{string}")) {
                    newNode = std::make_shared<TemplateNode<Value, "{string}">>();
                } else if (segment.starts_with("{int}")) {
                    newNode = std::make_shared<TemplateNode<Value, "{int}">>();
                } else if (segment.starts_with("{path}")) {
                    newNode = std::make_shared<TemplateNode<Value, "{path}">>();
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
        // It would be nice if this could be caught at compile time, but not sure how many more constexpr data objects
        // that would require
        if (node->value.contains(method)) {
            [[unlikely]]
            throw std::runtime_error("Illegal multiple use of the same route with the same method");
        }
        node->value[method] = value;
    }
};

}
