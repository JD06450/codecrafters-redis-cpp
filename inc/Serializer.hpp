#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_SERIALIZER_
#define _BUILD_YOUR_OWN_REDIS_SERIALIZER_

#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>

std::vector<uint8_t> from_json(const nlohmann::json &obj);

#endif
