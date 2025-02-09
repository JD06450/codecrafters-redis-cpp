#include "Commands.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <optional>

#include "Parser.hpp"
#include "ServerState.hpp"
#include "Store.hpp"

namespace json = nlohmann;

static const std::map<std::string, std::function<json::json(const json::json&, std::shared_ptr<Event>)>> commands =
{
	{"PING", PING},
	{"ECHO", ECHO},
	{"GET", GET},
	{"SET", SET},
	{"INFO", INFO},
	{"REPLCONF", REPLCONF},
	{"PSYNC", PSYNC}
};

json::json run_command(const json::json& command, std::shared_ptr<Event> ev)
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
		return it->second(args, ev);
	}

	std::cerr << "Command Not Found!\n";
	return json::json();
}

json::json PING(const json::json &_args, std::shared_ptr<Event> _ev)
{
	(void) _args;
	(void) _ev;
	return json::json("PONG");
}

json::json ECHO(const nlohmann::json &args, std::shared_ptr<Event> _ev)
{
	(void) _ev;
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

json::json GET(const json::json &args, std::shared_ptr<Event> _ev)
{
	(void) _ev;
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

bool parse_expiry_time(RedisSetOptions &options, const json::json &args, json::detail::iter_impl<const nlohmann::json> &it, const std::string &arg)
{
	options.data.expires = true;
	if ((it + 1 == args.end()) || !blob_is_numeric( *(++it) ))
	{
		std::cerr << "Flag " << arg << " requires a numerical argument.\n";
		return false;
	}
	std::vector<uint8_t> ttl_vec = it->get_binary();
	ssize_t ttl = std::stol(std::string(ttl_vec.cbegin(), ttl_vec.cend()), nullptr, 10);
	options.data.expiry_time = std::chrono::system_clock::now() + (arg == "PX" ? std::chrono::milliseconds(ttl) : std::chrono::seconds(ttl));
	return true;
}

json::json SET(const json::json &args, std::shared_ptr<Event> _ev)
{
	(void) _ev;
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

	for (auto it = args.begin() + 2; it != args.end(); ++it)
	{
		std::vector<uint8_t> arg_vec = it->get_binary();
		std::string arg(arg_vec.cbegin(), arg_vec.cend());
		std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
		bool status;
		if (arg == "PX" || arg == "EX") status = parse_expiry_time(options, args, it, arg);
		if (!status) return json::json();
	}

	bool result = RedisStore::get_store()->set_value(options);
	if (result) return json::json("OK");
	else return json::json();
}

json::json replication_info()
{
	std::shared_ptr<ServerState> state = ServerState::get_state();

	std::ostringstream out_stream;
	out_stream << "# Replication\r\n"
		<< "role:" << (state->replica_mode ? "slave" : "master") << "\r\n"
		<< "master_replid:" << (state->replication_id) << "\r\n"
		<< "master_repl_offset:" << (state->replication_offset) << "\r\n";
	return json::json(out_stream.str());
}

inline void append_blob(json::json &json_blob, const std::string &string_to_append)
{
	std::vector<uint8_t> a = json_blob.get_binary();
	std::vector<uint8_t> vec_to_append(string_to_append.cbegin(), string_to_append.cend());
	a.insert(a.end(), vec_to_append.cbegin(), vec_to_append.cend());
	json_blob = json::json::binary(a);
}

json::json INFO(const json::json &args, std::shared_ptr<Event> _ev)
{
	(void) _ev;
	json::json return_string = json::json::binary({});

	if (args.size() == 0)
	{
		// Concatenate all sections
		append_blob(return_string, replication_info());

		return return_string;
	}

	for (auto it = args.begin(); it != args.end(); ++it)
	{
		std::vector<uint8_t> arg_vec = it->get_binary();
		std::string arg(arg_vec.cbegin(), arg_vec.cend());
		std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
		if (arg == "replication") append_blob(return_string, replication_info());
	}

	return return_string;
}

json::json REPLCONF(const json::json &_args, std::shared_ptr<Event> _ev)
{
	(void) _args;
	(void) _ev;
	return json::json("OK");
}

json::json PSYNC(const json::json &args, std::shared_ptr<Event> ev)
{
	std::shared_ptr<ServerState> state = ServerState::get_state();
	std::ostringstream response;
	response << "FULLRESYNC " << state->replication_id << " " << state->replication_offset;
	return response.str();
}