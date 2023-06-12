#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace maya {

	class config {
	public:
		//inline static const std::string PATH_TO_TELEGRAM_JSON{ "../../secrets/telegram_secrets.json" }; // running inside visual studio -> fixed in CMakeLists!
		inline static const std::string PATH_TO_TELEGRAM_JSON{ "../../../secrets/telegram_secrets.json" }; // running from script dir (or any directory at the same depth)
	};

	class telegram_config {
	public:
		const std::string bot_secret;
		const long long main_chat_id;

		telegram_config(const nlohmann::json& telegram_json) :
			bot_secret(telegram_json.at("bot_secret")),
			main_chat_id(telegram_json.at("main_chat_id"))
		{}
	};
}
