#include "Response.hpp"
#include "magpie/transfer/adapters/FixedDataAdapter.hpp"
#include "CompressedResponse.hpp"

namespace magpie {

Response::Response() : code(&Status::OK), body(nullptr) {}
Response::Response(
    const StatusCode& code,
    std::string&& body,
    std::string&& contentType
) : 
    code(&code),
    body(std::make_shared<FixedDataAdapter>(std::move(body))),
    contentType(std::move(contentType)) 
{}
Response::Response(
    const StatusCode& code,
    std::shared_ptr<DataAdapter>&& bodyAdapter,
    std::string&& contentType
) :
    code(&code),
    body(std::move(bodyAdapter)),
    contentType(std::move(contentType))

{}

void Response::setBody(std::string&& body) {
    this->body = std::make_shared<FixedDataAdapter>(std::move(body));
}

Response& Response::operator=(CompressedResponse&& other) {
    this->headers = std::move(other.headers);
    this->code = other.code;
    this->body = std::move(other.body);
    this->contentType = std::move(other.contentType);

    return *this;
}

}
