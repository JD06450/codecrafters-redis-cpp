#include "Store.hpp"

using std::nullopt;

std::shared_ptr<RedisStore> RedisStore::get_store()
{
	static auto instance = std::shared_ptr<RedisStore>(new RedisStore());
	return instance;
}

RedisStore::~RedisStore() {}



bool RedisStore::set_value(const RedisSetOptions &options)
{
	std::lock_guard<std::mutex> g_data_mutex(this->data_mutex);
	auto it = this->data.find(options.key);
	if (options.assign_only && it == this->data.end()) return false;
	if (options.insert_only && it != this->data.end()) return false;
	this->data.insert_or_assign(it, options.key, options.data);
	return true;
}

std::optional<RedisValue> RedisStore::get_value(const std::string &key)
{
	auto it = this->data.find(key);
	if (it == this->data.end()) return nullopt;
	return it->second;
}
