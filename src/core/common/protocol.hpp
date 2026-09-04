#pragma once
#include <string>
#include <optional>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace libsorter::common {

enum class MsgType : uint8_t { Request, Response, Event };

struct WsMessage {
    MsgType type;
    std::string id;
    std::string action;
    nlohmann::json payload;
    std::optional<std::string> token;

    nlohmann::json to_json() const;
    static WsMessage from_json(const nlohmann::json& j);
};

enum class ErrorCode : uint16_t {
    Ok = 0, BadRequest = 400, Unauthorized = 401, Forbidden = 403,
    NotFound = 404, ValidationError = 422, InternalError = 500
};

struct ErrorResponse {
    ErrorCode code;
    std::string message;
    nlohmann::json to_json() const;
};

WsMessage make_response(const std::string& req_id, const nlohmann::json& payload);
WsMessage make_error_response(const std::string& req_id, ErrorCode code, const std::string& msg);
WsMessage make_event(const std::string& action, const nlohmann::json& payload);

} // namespace libsorter::common
