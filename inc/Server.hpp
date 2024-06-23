#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_SERVER_
#define _BUILD_YOUR_OWN_REDIS_SERVER_

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <array>
#include <vector>
#include <list>
#include <optional>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "Clients.hpp"

/**
 * @brief Accepts any incoming TCP connections for a given socket.
 * @param server_fd the file descriptor for the socket
 * @param connections a mutable reference to a list of active connections
 */
void accept_incoming_connections(int server_fd, std::list<client_connection> &connections);

std::optional<client_connection> accept_connection(int server_fd);

/**
 * @brief Closes sockets and deletes their entries in a list
 * @param socket_list a list of sockets
 * @param indexes a list of indexes into the socket list to close and remove
 */
void close_dead_sockets(std::list<client_connection> &socket_list, const std::vector<std::list<client_connection>::iterator> &indexes);

#endif