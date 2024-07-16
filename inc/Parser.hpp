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

bool blob_is_numeric(const nlohmann::json& blob);

/**
 * @brief Parses raw data from a TCP socket into a JSON object representing one or multiple Redis commands.
 * This function can be useful if the TCP socket sends multiple commands at once and as such, multiple commands
 * will have been read from the socket at once and cannot be easily separated without first parsing the data.
 * @param commands Raw command data streamed in from a TCP socket
 * @returns A JSON array of the parsed command data, where each array element is a specific command.
 */
nlohmann::json parse_redis_commands(const std::vector<uint8_t> &commands);

inline nlohmann::json parse_redis_item(const std::vector<uint8_t> &raw_item, size_t &item_length)
{
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

inline nlohmann::json parse_redis_item(const std::vector<uint8_t> &raw_item)
{
	size_t item_length = 0;
	return parse_redis_item(raw_item, item_length);
}

inline nlohmann::json parse_redis_command(const std::vector<uint8_t> &command)
{
	size_t _offset = 0;
	return parse_redis_item(command, _offset);
}

inline std::vector<uint8_t> to_blob(const std::string &str)
{
	return std::vector<uint8_t>(str.cbegin(), str.cend());
}

#endif