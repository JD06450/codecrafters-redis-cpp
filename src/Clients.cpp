#include "Clients.hpp"

#include <nlohmann/json.hpp>

namespace json = nlohmann;

// TODO: maybe see if there is a better way of choosing the buffer size
#define PACKET_BUFFER_LEN 4096

bool socket_ready(int fd, int timeout)
{
	pollfd poll_settings;
	poll_settings.fd = fd;
	poll_settings.events = POLLIN;
	poll(&poll_settings, 1, timeout);
	return poll_settings.revents & POLLIN;
}

inline bool socket_ready(int fd) {return socket_ready(fd, 1);}

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
	try
	{
		full_command = get_response(client);
	}
	catch(const std::runtime_error& e)
	{
		if (!strncmp(e.what(), "Disconnect", sizeof("Disconnect"))) return false;
		if (!strncmp(e.what(), "Socket Error", sizeof("Socket Error"))) return true;
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

	json::json cmds = parse_redis_commands(full_command);
	for (auto cmd = cmds.begin(); cmd != cmds.end(); ++cmd)
	{
		std::cout << "Command:  " << json::to_string(cmd.value()) << "\n\n";
		json::json response = run_command(cmd.value());
		std::cout << "Response: " << json::to_string(response) << "\n\n";
		std::vector<uint8_t> serial_response = from_json(response);

		send(client.fd, (const char *) serial_response.data(), serial_response.size(), 0);
	}

	return true;
}

std::vector<uint8_t> get_response(const client_connection &conn)
{
	std::vector<uint8_t> full_response;
	ssize_t bytes_read;
	std::array<char, PACKET_BUFFER_LEN> in_buffer;
	in_buffer.fill('\00');

	while (socket_ready(conn.fd))
	{
		bytes_read = read(conn.fd, in_buffer.data(), in_buffer.max_size());
		if (bytes_read == 0)
		{
			// EOF
			char client_address[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(conn.addr.sin_addr), client_address, INET_ADDRSTRLEN);
			std::cout << "Client at \"" << client_address << "\" on port " << conn.addr.sin_port << " disconnected\n";
			throw std::runtime_error("Disconnect");
		}
		if (bytes_read == -1)
		{
			std::cout << "Error reading from client\n";
			// TODO: maybe send error message back to the client if possible
			throw std::runtime_error("Socket Error");
		}

		full_response.insert(full_response.end(), in_buffer.data(), in_buffer.data() + bytes_read);
	}

	return full_response;
}