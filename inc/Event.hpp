#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_EVENT_
#define _BUILD_YOUR_OWN_REDIS_EVENT_

#include <functional>
#include <optional>
#include <memory>

#include "Clients.hpp"

struct Context
{
	client_connection client;
};

struct Event
{
	Context context;
	std::function<void (std::shared_ptr<Event>)> action;
	std::function<bool (std::shared_ptr<Event>)> timeout_function;

	Event(
		std::function<void (std::shared_ptr<Event>)> a,
		std::function<bool (std::shared_ptr<Event>)> tf,
		Context c
	) : action(a), timeout_function(tf), context(c)
	{}

	Event(
		std::function<void (std::shared_ptr<Event>)> a,
		Context c
	);
};

inline bool no_timeout(std::shared_ptr<Event> _context) {(void) _context; return true;}

#endif