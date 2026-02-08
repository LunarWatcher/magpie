#include "Response.hpp"
#include "magpie/transfer/adapters/FixedDataAdapter.hpp"

namespace magpie {

Response::Response() : code(&Status::OK), body(nullptr) {}

Response::Response(const StatusCode& code, std::string&& body) 
    : code(&code), body(std::make_shared<FixedDataAdapter>(std::move(body))) {}
Response::Response(const StatusCode& code, std::string&& body, std::string&& contentType) 
    : 
    code(&code),
    body(std::make_shared<FixedDataAdapter>(std::move(body))),
    contentType(std::move(contentType)) 
{}

void Response::setBody(std::string&& body) {
    this->body = std::make_shared<FixedDataAdapter>(std::move(body));
}

}
