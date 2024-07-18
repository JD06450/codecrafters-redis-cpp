#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_SERIALIZER_
#define _BUILD_YOUR_OWN_REDIS_SERIALIZER_

#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>

std::vector<uint8_t> serialize_array(const json::json &obj);

std::vector<uint8_t> serialize_array_and_push(const json::json &obj, std::stack<json::json> &stack);

std::vector<uint8_t> serialize_blob(const json::json &obj);

std::vector<uint8_t> serialize_string(const json::json &obj);

std::vector<uint8_t> serialize_null(const json::json &obj);

std::vector<uint8_t> from_json(const nlohmann::json &obj);

#endif
