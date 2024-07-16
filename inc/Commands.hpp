#pragma once
#ifndef _BUILD_YOUR_OWN_REDIS_COMMANDS_
#define _BUILD_YOUR_OWN_REDIS_COMMANDS_

#include <nlohmann/json.hpp>

#include "Parser.hpp"
#include "ServerState.hpp"
#include "Store.hpp"

nlohmann::json run_command(const nlohmann::json& command);

nlohmann::json PING(const nlohmann::json& args);

nlohmann::json ECHO(const nlohmann::json& args);

nlohmann::json GET(const nlohmann::json &args);

nlohmann::json SET(const nlohmann::json &args);

nlohmann::json INFO(const nlohmann::json &args);

nlohmann::json REPLCONF(const nlohmann::json &args);

nlohmann::json PSYNC2(const nlohmann::json &args);

#endif