#include "Server.hpp"

namespace po = boost::program_options;
using boost::lexical_cast;

static const std::regex ipv4_regex("^(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})\\.(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})\\.(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})\\.(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})$");

std::optional<client_connection> accept_connection(int server_fd)
{
	client_connection client;
	socklen_t addr_size = sizeof(client.addr);
	client.fd = accept(server_fd, (sockaddr *)&client.addr, &addr_size);
	// int one = 1;

	if (client.fd < 0)
	{
		std::cerr << "Failed to get client socket\n";
		return std::nullopt;
	}

	// if (setsockopt(client.fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)) < 0)
	// {
	// 	std::cerr << "Failed to set socket option";
	// 	return;
	// }
	
	return client;
}

void accept_incoming_connections(int server_fd, std::list<client_connection> &connections)
{
	std::optional<client_connection> new_connection;
	pollfd poll_settings;
	poll_settings.fd = server_fd;
	poll_settings.events = POLLIN;

	int poll_status = 0;
	// Continuously check for incoming connections until all have been accepted.
	while ((poll_status = poll(&poll_settings, 1, 0)) > 0)
	{
		if ((new_connection = accept_connection(server_fd)).has_value())
			connections.push_back(new_connection.value());
	}

	if (poll_status < 0) std::cerr << "Failed to poll for incoming connections.\n";
}

void close_dead_sockets(std::list<client_connection> &socket_list, const std::vector<std::list<client_connection>::iterator> &indexes)
{
	if (indexes.size() == 0) return;

	for (auto idx : indexes)
	{
		close(idx->fd);
		socket_list.erase(idx);
	}
}

int main(int argc, char **argv) {
	// Flush after every std::cout / std::cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;
	
	po::options_description desc("Allowed Options");
	desc.add_options()
		("port", po::value<uint16_t>(), "The port to bind the server to")
		("replicaof", po::value<std::string>(), "Start the server in replica mode and replicate changes from a redis server at this address");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	std::shared_ptr<ServerState> state = ServerState::get_state();

	if (vm.count("port"))
	{
		state->port = vm.at("port").as<uint16_t>();
	}

	if (vm.count("replicaof"))
	{
		std::string value = vm.at("replicaof").as<std::string>();
		size_t idx = value.find(' ');
		std::string ip = value.substr(0, idx);
		std::string port_str = value.substr(idx + 1);

		if (ip == "localhost") ip = "127.0.0.1";
		if (!std::regex_match(ip, ipv4_regex) || inet_pton(AF_INET, ip.c_str(), &state->replica_addr.sin_addr) < 1)
		{
			throw std::range_error("Invalid IPv4 address in --replicaof");
		}
		
		state->replica_addr.sin_port = htons(lexical_cast<uint16_t>(port_str));
		state->replica_mode = true;
	}

	if (state->replica_mode == true)
	{
		state->master_fd = init_replica_socket();
		try
		{
			replica_handshake(state->master_fd);
		}
		catch(const std::runtime_error &e)
		{
			std::cerr << "Replica handshake failed. Reason: " << e.what() << '\n';
		}
	}

	// Create server socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Failed to create server socket\n";
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		std::cerr << "setsockopt failed\n";
		return 1;
	}
	
	// Binding the socket to port 6379
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(state->port);
	
	if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
		std::cerr << "Failed to bind to port "<< state->port <<"\n";
		return 1;
	}
	
	// Listen for connections with a maximum of 5
	int max_connections = 5;
	if (listen(server_fd, max_connections) != 0) {
		std::cerr << "listen failed\n";
		return 1;
	}
	
	client_connection initial_client;
	
	// Listening for our first client to start
	{
		socklen_t initial_client_addr_len = sizeof(initial_client.addr);
		std::cout << "Waiting for a client to connect...\n";	
		initial_client.fd = accept(server_fd, (struct sockaddr *) &initial_client.addr, &initial_client_addr_len);
	}


	if (initial_client.fd < 0)
	{
		std::cerr << "Failed to get client socket\n";
		return 1;
	}
	std::cout << "Client connected\n";

	std::list<client_connection> connections;
	std::vector<std::list<client_connection>::iterator> sockets_to_prune;
	fd_set connection_set;

	connections.push_back(initial_client);

	while (connections.size() > 0)
	{
		int max_fd = 0;

		close_dead_sockets(connections, sockets_to_prune);
		sockets_to_prune.clear();
		// Create list of file descriptors to check
		FD_ZERO(&connection_set);
		FD_SET(server_fd, &connection_set);
		for (auto client : connections)
		{
			max_fd = std::max(max_fd, client.fd);
			FD_SET(client.fd, &connection_set);
		}

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1;

		// Check the descriptors for activity
		int a = select(max_fd + 1, &connection_set, NULL, NULL, &timeout);

		if (FD_ISSET(server_fd, &connection_set)) accept_incoming_connections(server_fd, connections);

		for (auto client = connections.begin(); client != connections.end(); ++client)
		{
			if (!FD_ISSET(client->fd, &connection_set)) continue;
			if (!handle_client_read(*client)) sockets_to_prune.push_back(client);
		}
	}

	close(server_fd);

	return 0;
}