#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_SERIALIZER_
#define _BUILD_YOUR_OWN_REDIS_SERIALIZER_

#include <cstdint>
#include <stack>
#include <vector>

#include <nlohmann/json.hpp>

std::vector<uint8_t> serialize_array(const nlohmann::json &obj);

std::vector<uint8_t> serialize_array_and_push(const nlohmann::json &obj, std::stack<nlohmann::json> &stack);

std::vector<uint8_t> serialize_blob(const nlohmann::json &obj);

std::vector<uint8_t> serialize_string(const nlohmann::json &obj);

std::vector<uint8_t> serialize_null(const nlohmann::json &obj);

std::vector<uint8_t> from_json(const nlohmann::json &obj);

#endif
