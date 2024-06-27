#include "Clients.hpp"

#include <nlohmann/json.hpp>

namespace json = nlohmann;

// TODO: maybe see if there is a better way of choosing the buffer size
#define PACKET_BUFFER_LEN 4096

bool socket_ready(int fd)
{
	pollfd poll_settings;
	poll_settings.fd = fd;
	poll_settings.events = POLLIN;
	poll(&poll_settings, 1, 1);
	return poll_settings.revents & POLLIN;
}

void print_blob(const std::vector<uint8_t> &blob)
{
	for (const uint8_t &ch : blob)
	{
		std::cout << std::showbase << std::setw(2) << std::hex << (int) ch << ' ';
	}
	std::cout << std::endl;
}

bool handle_client_read(const client_connection& client)
{
	std::vector<uint8_t> full_command;
	std::array<char, PACKET_BUFFER_LEN> in_buffer;
	ssize_t bytes_read;
	in_buffer.fill('\00');

	while (socket_ready(client.fd))
	{
		bytes_read = read(client.fd, in_buffer.data(), in_buffer.max_size());
		if (bytes_read == 0)
		{
			// EOF
			char client_address[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client.addr.sin_addr), client_address, INET_ADDRSTRLEN);
			std::cout << "Client at \"" << client_address << "\" on port " << client.addr.sin_port << " disconnected\n";
			return false;
		}
		if (bytes_read == -1)
		{
			std::cout << "Error reading from client\n";
			// TODO: maybe send error message back to the client if possible
			return true;
		}

		full_command.insert(full_command.end(), in_buffer.data(), in_buffer.data() + bytes_read);
	}

	for (char i : full_command)
	{
		if (i == 0) break;
		switch (i) {
		case '\\':
			std::cout << "\\\\";
			break;
		case '\r':
			std::cout << "\\r";
			break;
		case '\n':
			std::cout << "\\n";
			// break;
		default:
			std::cout << i;
		}
	}

	json::json cmd = parse_redis_command(full_command);
	std::cout << "Command:  " << json::to_string(cmd) << "\n\n";
	json::json response = run_command(cmd);
	std::cout << "Response: " << json::to_string(response) << "\n\n";

	std::vector<uint8_t> serial_response = from_json(response);

	

	send(client.fd, (const char *) serial_response.data(), serial_response.size(), 0);
	return true;
}