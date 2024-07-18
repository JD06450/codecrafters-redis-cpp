#include "Event.hpp"

Event::Event(std::function<void(std::shared_ptr<Event>)> a, Context c) :
	action(a),
	context(c)
{
	this->timeout_function = no_timeout;
}
