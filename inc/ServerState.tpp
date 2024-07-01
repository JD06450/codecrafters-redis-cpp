#include "ServerState.hpp"

template<typename T>
requires std::integral<T>
T ServerState::random()
{
	std::uniform_int_distribution<T> dist;
	return dist(this->rng);
}