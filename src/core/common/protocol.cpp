#include "common/protocol.hpp"
#include <stdexcept>

namespace libsorter::common {
using json = nlohmann::json;

static std::string msg_type_to_string(MsgType t) {
    switch (t) {
        case MsgType::Request:  return "request";
        case MsgType::Response: return "response";
        case MsgType::Event:    return "event";
    }
    return "unknown";
}

static MsgType msg_type_from_string(const std::string& s) {
    if (s == "request")  return MsgType::Request;
    if (s == "response") return MsgType::Response;
    if (s == "event")    return MsgType::Event;
    throw std::invalid_argument("Unknown MsgType: " + s);
}

json WsMessage::to_json() const {
    json j = {{"type", msg_type_to_string(type)}, {"id", id}, {"action", action}, {"payload", payload}};
    if (token.has_value()) j["token"] = *token;
    return j;
}

WsMessage WsMessage::from_json(const json& j) {
    WsMessage msg;
    msg.type = msg_type_from_string(j.at("type").get<std::string>());
    msg.id = j.at("id").get<std::string>();
    msg.action = j.at("action").get<std::string>();
    msg.payload = j.value("payload", json::object());
    if (j.contains("token")) msg.token = j["token"].get<std::string>();
    return msg;
}

json ErrorResponse::to_json() const {
    return {{"code", static_cast<uint16_t>(code)}, {"message", message}};
}

WsMessage make_response(const std::string& req_id, const json& payload) {
    return {MsgType::Response, req_id, "", payload, std::nullopt};
}

WsMessage make_error_response(const std::string& req_id, ErrorCode code, const std::string& msg) {
    return {MsgType::Response, req_id, "", ErrorResponse{code, msg}.to_json(), std::nullopt};
}

WsMessage make_event(const std::string& action, const json& payload) {
    return {MsgType::Event, "", action, payload, std::nullopt};
}

} // namespace libsorter::common
