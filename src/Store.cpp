#include "Store.hpp"

using std::nullopt;

std::shared_ptr<RedisStore> RedisStore::get_store()
{
	static auto instance = std::shared_ptr<RedisStore>(new RedisStore());
	return instance;
}

RedisStore::~RedisStore() {}


void RedisStore::check_expiry(const std::string &key)
{
	auto it = this->data.find(key);
	if (it == this->data.end()) return;
	RedisValue data = it->second;
	if (data.expires && std::chrono::system_clock::now() >= data.expiry_time)
	{
		this->data.erase(it);
	}
}


bool RedisStore::set_value(const RedisSetOptions &options)
{
	this->check_expiry(options.key);

	auto it = this->data.find(options.key);
	if (options.assign_only && it == this->data.end()) return false;
	if (options.insert_only && it != this->data.end()) return false;
	this->data.insert_or_assign(it, options.key, options.data);
	return true;
}

std::optional<RedisValue> RedisStore::get_value(const std::string &key)
{
	this->check_expiry(key);
	
	auto it = this->data.find(key);
	if (it == this->data.end()) return nullopt;
	return it->second;
}
