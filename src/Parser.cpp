#include <algorithm>

#include "Parser.hpp"
namespace json = nlohmann;

json::json parse_redis_array(const std::vector<uint8_t> &raw_array, size_t &item_length)
{
	if (raw_array[0] != (uint8_t) data_type_ids::ARRAY) return json::json();

	size_t raw_index = 0, el_length = 0;
	json::json parsed_array = json::json::array();
	ssize_t array_len;

	{
		std::vector<uint8_t>::const_iterator it = std::search(raw_array.cbegin(), raw_array.cend(), terminator.cbegin(), terminator.cend());
		if (it == raw_array.cend()) return json::json();
		std::string str_array(raw_array.cbegin() + 1, it);
		array_len = std::stol(str_array, &raw_index, 10);
		raw_index++;
	}

	raw_index += 2;

	for (int parsed_idx = 0; parsed_idx < array_len; ++parsed_idx)
	{
		json::json temp_item = parse_redis_item(std::vector<uint8_t>(raw_array.begin() + raw_index, raw_array.end()), el_length);
		parsed_array.push_back(std::move(temp_item));
		raw_index += el_length;
	}

	item_length = raw_index;

	return parsed_array;
}

json::json parse_redis_blob(const std::vector<uint8_t>& raw_blob, size_t &blob_length)
{
	if (raw_blob[0] != (uint8_t) data_type_ids::BLOB) return json::json();

	json::json parsed_blob = json::json::binary(std::vector<uint8_t>());
	size_t raw_index = 0;
	ssize_t blob_data_length;

	{
		std::vector<uint8_t>::const_iterator it = std::search(raw_blob.cbegin(), raw_blob.cend(), terminator.cbegin(), terminator.cend());
		if (it == raw_blob.cend()) return json::json();
		std::string blob_size_str(raw_blob.cbegin() + 1, it);
		blob_data_length = std::stol(blob_size_str, &raw_index, 10);
		raw_index++;
	}

	raw_index += 2;

	blob_length = raw_index + blob_data_length + 2;

	ssize_t diff = std::distance(raw_blob.begin() + blob_length - 2, raw_blob.end());

	return json::json::binary(std::vector<uint8_t>(raw_blob.begin() + raw_index, std::min(raw_blob.begin() + blob_length - 2, raw_blob.end() - 2)));
}

json::json parse_redis_string(const std::vector<uint8_t>& raw_str, size_t &str_length)
{
	if (raw_str[0] != (uint8_t) data_type_ids::STRING) return json::json();

	json::json parsed_string = "";
	size_t raw_index = 0;
	std::vector<uint8_t>::const_iterator it;

	{
		it = std::search(raw_str.cbegin(), raw_str.cend(), terminator.cbegin(), terminator.cend());
		if (it == raw_str.cend()) return json::json();
		parsed_string = std::string(raw_str.cbegin() + 1, it);
		raw_index++;
	}

	raw_index += parsed_string.get<std::string>().length();
	raw_index += 2;
	str_length = raw_index;

	return parsed_string;
}

bool blob_is_string_like(const json::json &blob_json)
{
	std::vector<uint8_t> blob = blob_json.get_binary();
	std::vector<uint8_t>::const_iterator it = std::find_if_not(blob.cbegin(), blob.cend(), [](uint8_t c)
	{
		return std::isalnum(c);
	});

	return it == blob.cend();
}

bool blob_is_numeric(const json::json &blob_json)
{
	std::vector<uint8_t> blob = blob_json.get_binary();
	std::vector<uint8_t>::const_iterator it = std::find_if_not(blob.cbegin(), blob.cend(), [](uint8_t c)
	{
		return std::isdigit(c);
	});

	return it == blob.cend();
}

json::json parse_redis_commands(const std::vector<uint8_t> &commands)
{
	json::json commands_array = json::json::array();
	size_t offset = 0;

	while (offset < commands.size())
	{
		size_t command_length = 0;
		std::vector<uint8_t> command(commands.begin() + offset, commands.end());
		commands_array.push_back(parse_redis_item(command, command_length));
		offset += command_length;
	}
	return commands_array;
}