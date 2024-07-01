#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_STATE_
#define _BUILD_YOUR_OWN_REDIS_STATE_

#include <type_traits>
#include <concepts>
#include <cstdint>
#include <memory>
#include <random>
#include <arpa/inet.h>

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
	bool replica_mode;
	struct sockaddr_in replica_addr;
	std::string replication_id;
	std::string replication_id_2;
	uint64_t replication_offset;
};

#include "ServerState.tpp"

#endif