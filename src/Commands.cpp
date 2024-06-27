#include "Commands.hpp"

#include <iostream>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <optional>

// #include "Store.hpp"

namespace json = nlohmann;

static const std::map<std::string, std::function<json::json(const json::json&)>> commands =
{
	{"PING", PING},
	{"ECHO", ECHO},
	{"GET", GET},
	{"SET", SET}
};

json::json run_command(const json::json& command)
{
	if (!blob_is_string_like(command.at(0)))
	{
		std::cerr << "Invalid Command" << std::endl;
	}

	std::vector<uint8_t> binary_cmd = command.at(0).get_binary();
	std::string cmd_string(binary_cmd.cbegin(), binary_cmd.cend());
	std::transform(cmd_string.begin(), cmd_string.end(), cmd_string.begin(), ::toupper);
	
	decltype(commands)::const_iterator it;
	if ((it = commands.find(cmd_string)) != commands.end())
	{
		json::json args(command.cbegin() + 1, command.cend());
		return it->second(args);
	}

	std::cerr << "Command Not Found!\n";
	return json::json();
}

json::json PING(const json::json &_args)
{
	return json::json("PONG");
}

nlohmann::json ECHO(const nlohmann::json &args)
{
	if (!args.is_array())
	{
		std::cerr << "ECHO command expects an array as input.\n";
		return json::json(std::vector<uint8_t>());
	}
	// TODO: send error back to client
	if (args.size() != 1)
	{
		std::cerr << "Wrong number of arguments supplied\n";
	}
	std::vector<uint8_t> response = {};

	if (args.at(0).is_binary()) response = args.at(0).get_binary();
	else if (args.at(0).is_string())
	{
		std::string in_string = args.at(0).get<std::string>();
		response.insert(response.cend(), in_string.cbegin(), in_string.cend());
	}
	else
	{
		std::cerr << "invalid argument type\n";
		return json::json(std::vector<uint8_t>());
	}
	
	return json::json::binary(response);
}

json::json GET(const json::json &args)
{
	if (!args.is_array())
	{
		std::cerr << "GET command expects an array as input.\n";
		return json::json(std::vector<uint8_t>());
	}

	std::vector<uint8_t> arg1 = args.at(0).get_binary();
	std::string key(arg1.cbegin(), arg1.cend());

	std::optional<RedisValue> value = RedisStore::get_store()->get_value(key);
	if (value.has_value()) return json::json::binary(value.value().data);
	else return json::json();
}

json::json SET(const json::json &args)
{
	if (!args.is_array())
	{
		std::cerr << "SET command expects an array as input.\n";
		return json::json(std::vector<uint8_t>());
	}

	RedisSetOptions options;

	{
		std::vector<uint8_t> arg = args.at(0).get_binary();
		options.key = std::string(arg.cbegin(), arg.cend());
		options.data.data = args.at(1).get_binary();
	}

	bool result = RedisStore::get_store()->set_value(options);
	if (result) return json::json("OK");
	else return json::json();
}