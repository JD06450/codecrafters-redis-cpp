#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_COMMANDS_
#define _BUILD_YOUR_OWN_REDIS_COMMANDS_

#include "Event.hpp"

#include <nlohmann/json.hpp>

nlohmann::json run_command(const nlohmann::json& command, std::shared_ptr<Event> event);

nlohmann::json PING(const nlohmann::json& args, std::shared_ptr<Event> event);

nlohmann::json ECHO(const nlohmann::json& args, std::shared_ptr<Event> event);

nlohmann::json GET(const nlohmann::json &args, std::shared_ptr<Event> event);

nlohmann::json SET(const nlohmann::json &args, std::shared_ptr<Event> event);

nlohmann::json INFO(const nlohmann::json &args, std::shared_ptr<Event> event);

nlohmann::json REPLCONF(const nlohmann::json &args, std::shared_ptr<Event> event);

nlohmann::json PSYNC(const nlohmann::json &args, std::shared_ptr<Event> event);

#endif