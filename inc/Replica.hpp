#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_REPLICA_
#define _BUILD_YOUR_OWN_REDIS_REPLICA_

/**
 * @brief Attempts to create a connection to the master server
 * @return A file descriptor to the new socket
 */
int init_replica_socket();

void replica_handshake(int socket);

#endif