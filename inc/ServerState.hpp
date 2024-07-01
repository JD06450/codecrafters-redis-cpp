#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_STATE_
#define _BUILD_YOUR_OWN_REDIS_STATE_

#include <memory>
#include <cstdint>
#include <arpa/inet.h>

class ServerState
{
private:
	ServerState();

public:
	static std::shared_ptr<ServerState> get_state();

	ServerState(const ServerState &) = delete;
	void operator=(const ServerState &) = delete;
	~ServerState();
	
	uint16_t port;
	struct sockaddr_in replica_addr;
	bool replica_mode;
};

#endif