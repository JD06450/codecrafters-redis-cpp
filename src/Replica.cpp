#include "Replica.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
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

	int reuse = 1;
	if (setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		throw std::runtime_error("Failed to set socket option");
	}

	if (connect(master_fd, (sockaddr *) &state->replica_addr, sizeof(state->replica_addr)) < 0)
	{
		throw std::runtime_error("Failed to connect to the master DB");
	}

	return master_fd;
}

void replica_handshake(int socket)
{
	json::json command = json::json::array();
	command.push_back(json::json::binary(to_blob("PING")));
	std::vector<uint8_t> serial_command = from_json(command);

	send(socket, (const char *) serial_command.data(), serial_command.size(), 0);

}