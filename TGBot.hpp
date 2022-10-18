// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include <Windows.h>
#include <wininet.h>
#include <string>
#include <vector>

#define DEFAULT_USERAGENT "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1"


// The library doesn't use any JSON parser, which might seem idiotic at first and unsafe, but I needed it to be small:)
class TGBot
{
#ifdef _DEBUG
#define DbgPrint(x) printf(x)
#define DbgPrintf(x, y) printf(x, y)
#endif

#ifndef _DEBUG
#define DbgPrint(x)
#define DbgPrintf(x, y)
#endif

	const char* FORM_URL_ENCODED = "Content-Type: application/x-www-form-urlencoded\r\n";
	const char* MULTIPART_FORM_DATA = "Content-Type: multipart/form-data";

	struct tg_message
	{
		uint64_t update_id{}, chat_id{}, date{};
		std::string username;
		std::string text;
	};

	bool m_bThrow = true;

	uint64_t lastUpdateId{0};

	// server addresses and tokens
	std::string apiAddress{"api.telegram.org"};
	std::string objectName{"/bot"};
	std::string api_token;

	// internal HINTERNET handles
	HINTERNET m_hOpen{};
	HINTERNET m_hConnect{};

	std::string makeQuery(const std::string&, const std::string& verb, const char* content_type, const std::string& parameters = "");

	tg_message parseMessageResponseObject(const std::string_view& str);


	void parseFirstOfField(const std::string_view&, std::string&, std::string_view&&);

	void clearMessageQueue();
	std::string makeGetUpdateRequest();
public:
	inline static bool parse_error{false};

	explicit TGBot(const std::string& token, bool autoConnect = true, bool throwOnError = true)
		: m_bThrow(throwOnError), api_token(token)
	{
		objectName += token + "/";
		if (autoConnect)
		{
			connectToBot();
		}
	}

	~TGBot()
	{
		closeAllConnections();
	}

	[[nodiscard]] std::vector<tg_message> getUpdates();

	bool connectToBot();
	void closeAllConnections() const;
	bool sendMessage(const uint64_t&, const std::string&);
};
