#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_STATE_
#define _BUILD_YOUR_OWN_REDIS_STATE_

#include <concepts>
#include <cstdint>
#include <list>
#include <memory>
#include <random>
#include <type_traits>
#include <arpa/inet.h>

#include "Event.hpp"

class ServerState
{
private:
	ServerState();
	std::random_device rd;
	std::mt19937 rng;

public:
	static std::shared_ptr<ServerState> get_state();

	template <typename T>
	requires std::integral<T>
	T random();

	ServerState(const ServerState &) = delete;
	void operator=(const ServerState &) = delete;
	~ServerState();
	
	uint16_t port;
	std::list<std::shared_ptr<Event>> events;
	bool replica_mode;

	int master_fd;
	struct sockaddr_in replica_addr;
	std::string replication_id;
	std::string replication_id_2;
	uint64_t replication_offset;
};

#include "ServerState.tpp"

#endif