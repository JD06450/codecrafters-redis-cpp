#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_PARSER_
#define _BUILD_YOUR_OWN_REDIS_PARSER_

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// #include "Types.hpp"

#include <nlohmann/json.hpp>

// https://redis.io/docs/latest/develop/reference/protocol-spec/

enum class data_type_ids
{
	ARRAY = '*',
	BLOB = '$', // a.k.a: Bulk String
	STRING = '+' // a.k.a: Simple String
};

constexpr const std::array<uint8_t, 2> terminator = {'\r', '\n'};

nlohmann::json parse_redis_array(const std::vector<uint8_t>& array, size_t &array_length);

nlohmann::json parse_redis_blob(const std::vector<uint8_t>& blob, size_t &blob_length);

nlohmann::json parse_redis_string(const std::vector<uint8_t>& str, size_t &str_length);

bool blob_is_string_like(const nlohmann::json& blob);

inline nlohmann::json parse_redis_item(const std::vector<uint8_t> &raw_item, size_t &item_length)
{
	// std::cout << "Item:\n";
	// for (char i : raw_item)
	// {
	// 	if (i == 0) break;
	// 	switch (i) {
	// 	case '\\':
	// 		std::cout << "\\\\";
	// 		break;
	// 	case '\r':
	// 		std::cout << "\\r";
	// 		break;
	// 	case '\n':
	// 		std::cout << "\\n";
	// 		break;
	// 	default:
	// 		std::cout << i;
	// 	}
	// }
	// std::cout << "\n";

	switch (raw_item[0])
	{
	case (uint8_t) data_type_ids::ARRAY:
		return parse_redis_array(raw_item, item_length);
	case (uint8_t) data_type_ids::BLOB:
		return parse_redis_blob(raw_item, item_length);
	case (uint8_t) data_type_ids::STRING:
		return parse_redis_string(raw_item, item_length);
	default:
		return nlohmann::json();
	}
}

inline nlohmann::json parse_redis_command(const std::vector<uint8_t> &command)
{
	size_t _offset = 0;
	return parse_redis_item(command, _offset);
}

#endif