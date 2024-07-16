#include "Replica.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

namespace json = nlohmann;

int init_replica_socket()
{
	std::shared_ptr<ServerState> state = ServerState::get_state();
	if (state->replica_addr.sin_addr.s_addr == INADDR_NONE)
	{
		throw std::runtime_error("Invalid master DB address");
	}

	int master_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (master_fd < 0)
	{
		throw std::runtime_error("Failed to create socket to master db");
	}

	int flags = 1;
	if (setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags)) < 0)
	{
		throw std::runtime_error("Failed to set socket option");
	}

	// if (setsockopt(master_fd, SOL_TCP, TCP_NODELAY, &flags, sizeof(flags)) < 0)
	// {
	// 	throw std::runtime_error("Failed to set socket option");
	// }

	if (connect(master_fd, (sockaddr *) &state->replica_addr, sizeof(state->replica_addr)) < 0)
	{
		throw std::runtime_error("Failed to connect to the master DB");
	}

	return master_fd;
}

void replica_handshake(int socket)
{
	json::json command = json::json::array();
	json::json response;
	command.push_back(json::json::binary(to_blob("PING")));
	std::vector<uint8_t> serial_command = from_json(command);

	client_connection master_conn;
	master_conn.fd = socket;
	master_conn.addr = ServerState::get_state()->replica_addr;

	send(socket, (const char *) serial_command.data(), serial_command.size(), 0);
	// TODO: This is bad. Don't do this
	while (!socket_ready(socket, 3000)) {continue;}
	response = parse_redis_item(get_response(master_conn));
	if (response != json::json("PONG"))
	{
		throw std::runtime_error("Handshake failed stage 1");
	}

	command = json::json::array({
		json::json::binary(to_blob("REPLCONF")),
		json::json::binary(to_blob("listening-port")),
		json::json::binary(to_blob(std::to_string(ServerState::get_state()->port)))
	});
	serial_command = from_json(command);

	send(socket, (const char *) serial_command.data(), serial_command.size(), 0);
	while (!socket_ready(socket, 3000)) {continue;}
	response = parse_redis_item(get_response(master_conn));
	if (response != json::json("OK"))
	{
		throw std::runtime_error("Handshake failed stage 2");
	}
	
	command = json::json::array({
		json::json::binary(to_blob("REPLCONF")),
		json::json::binary(to_blob("capa")),
		json::json::binary(to_blob("eof")),
		json::json::binary(to_blob("capa")),
		json::json::binary(to_blob("psync2"))
	});
	serial_command = from_json(command);

	send(socket, (const char *) serial_command.data(), serial_command.size(), 0);
	while (!socket_ready(socket, 3000)) {continue;}
	response = parse_redis_item(get_response(master_conn));
	if (response != json::json("OK"))
	{
		throw std::runtime_error("Handshake failed stage 3");
	}

	command = json::json::array({
		json::json::binary(to_blob("PSYNC2")),
		json::json::binary(to_blob("?")),
		json::json::binary(to_blob("-1"))
	});
	serial_command = from_json(command);

	send(socket, (const char *) serial_command.data(), serial_command.size(), 0);
	while (!socket_ready(socket, 3000)) {continue;}
	response = parse_redis_item(get_response(master_conn));
	std::vector<std::string> tokens;
	{
		std::istringstream i(response.get<std::string>());
		std::copy(
			std::istream_iterator<std::string>(i),
			std::istream_iterator<std::string>(),
			std::back_inserter(tokens)
		);
	}

	if (tokens.size() != 3)
	{
		std::ostringstream what;
		what << "Expected 3 tokens, got " << tokens.size();
		throw std::runtime_error(what.str());
	}

	std::shared_ptr<ServerState> state = ServerState::get_state();
	state->replication_id = tokens[1];
	state->replication_offset = std::stoull(tokens[2]);
}