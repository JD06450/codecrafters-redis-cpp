#include "ServerState.hpp"

#include <cstring>

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
}

ServerState::~ServerState() {}
