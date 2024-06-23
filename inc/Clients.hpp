#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_CLIENTS_
#define _BUILD_YOUR_OWN_REDIS_CLIENTS_

#include <sys/socket.h>
#include <arpa/inet.h>

struct client_connection
{
	int fd;
	sockaddr_in addr;
};



#endif