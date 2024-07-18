#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_STORE_
#define _BUILD_YOUR_OWN_REDIS_STORE_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <memory>
#include <optional>

using system_time_point = std::chrono::time_point<std::chrono::system_clock>;

struct RedisValue
{
	std::vector<uint8_t> data;
	system_time_point expiry_time = system_time_point::max();
	bool expires = false;
};

struct RedisSetOptions
{
	std::string key;
	RedisValue data;
	bool insert_only = false;
	bool assign_only = false;
	bool keep_ttl = false;
};

class RedisStore
{
private:
	std::unordered_map<std::string, RedisValue> data;

	RedisStore() {};

public:
	static std::shared_ptr<RedisStore> get_store();
	
	RedisStore(const RedisStore &) = delete;
	void operator=(const RedisStore &) = delete;
	~RedisStore();

	void check_expiry(const std::string &key);

	bool set_value(const RedisSetOptions &options);

	std::optional<RedisValue> get_value(const std::string &key);
};

#endif