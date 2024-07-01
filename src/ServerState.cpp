#include "ServerState.hpp"

#include <cstring>
#include <sstream>
#include <iomanip>

std::shared_ptr<ServerState> ServerState::get_state()
{
	static auto instance = std::shared_ptr<ServerState>(new ServerState());
	return instance;
}

ServerState::ServerState()
{
	memset(&this->replica_addr, 0, sizeof(this->replica_addr));
	this->replica_addr.sin_addr.s_addr = INADDR_NONE;
	this->replica_addr.sin_port = 0;
	this->replica_addr.sin_family = AF_INET;
	this->replica_mode = false;
	this->port = 6379;

	this->rng = std::mt19937(this->rd());

	this->replication_offset = 0;
	this->replication_id_2 = "0000000000000000000000000000000000000000";

	std::ostringstream id_stream;

	// Generating 20 random bytes, or 160 random bits
	// for the replication ID
	for (int i = 0; i < 5; i++)
	{
		uint32_t random_int = this->random<uint32_t>();
		id_stream << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex << random_int;
	}

	this->replication_id = id_stream.str();
}

ServerState::~ServerState() {}
