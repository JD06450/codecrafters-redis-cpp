#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_CLIENTS_
#define _BUILD_YOUR_OWN_REDIS_CLIENTS_

#include <array>
#include <cstdint>
#include <vector>

struct client_connection
{
	int fd;
	sockaddr_in addr;
};

bool socket_ready(int fd);

bool socket_ready(int fd, int timeout);

/**
 * @brief 
 * @param client The client to handle
 * @return True if the socket should be kept alive, False if the socket should be closed
 */
bool handle_client_read(const client_connection &client);

void print_command(const std::vector<uint8_t> &full_command);

std::vector<uint8_t> get_response(const client_connection &conn);

#endif