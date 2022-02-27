#pragma once
#ifndef TG_INCLUDES
#define TG_INCLUDES
#include <Windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#endif

class TGBot
{
	struct tg_message
	{
		uint64_t update_id{}, chat_id{}, date{};
		std::string username;
		std::string text;
	};


	uint64_t lastUpdateId{0};

	std::string urlQueryString{"https://api.telegram.org/bot"};

	std::string api_token;

	static std::string makeUrlQuery(const std::string&);

	static tg_message parseMessageObject(const std::string_view& str);


	static void parseUntil(const std::string_view&, std::string&, std::string_view&&, const size_t& offset = 0,
	                       const char& separator = ',');

	void clearMessageQueue() const;

public:
	inline static bool internet_error{false}, parse_error{false};

	explicit TGBot(const std::string& token)
		: api_token(token)
	{
		urlQueryString += token + "/";
	}

	[[nodiscard]] std::vector<tg_message> getUpdates();

	void sendMessage(const uint64_t&, const std::string&) const;
};
