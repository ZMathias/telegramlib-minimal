// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "TGBot.hpp"

#include <stdexcept>

std::string TGBot::makeQuery(const std::string& apiMethod, const std::string& verb, const char* content_type, const std::string& parameters)
{
	const char* accept[] = { "*/*", nullptr };

	// we assemble the rest of the url with parameters based on the verb
	// techically we could just use a GET request for everything
	// and encode every parameter in the url, but this is more
	// flexible and allows us to use POST requests for some methods
	// like uploading files
	// we do it like this because the telegram api doesn't accept x-www-form-urlencoded
	// in a GET request, only POST
	std::string finalObjectName = objectName + apiMethod;

	if (verb == "GET" && !parameters.empty()) {
		finalObjectName += "?" + parameters;
	}

	// HTTPS is required for the telegram api
	// otherwise it just returns 403 bad request and doesn't tell you otherwise
	const HINTERNET open_handle = HttpOpenRequestA(
		m_hConnect,
		verb.c_str(),
		finalObjectName.c_str(),
		"HTTP/1.1",
		nullptr,
		accept,
		INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_RESYNCHRONIZE,
		0
		);

	if (open_handle == nullptr)
	{
		DbgPrint("HttpOpenRequestA failed");
		return {};
	}

	BOOL success = false;

	if (verb == "POST")
	{
		// pass the optional parameters as the request body in the case of a POST request
		// we could pass it all the time, but if the body is somehow large we waste bandwidth
		success = HttpSendRequestA(
			open_handle,
			content_type,
			-1L,
			const_cast<char*>(parameters.c_str()), // the analyzer complained about C casting, no other point in it
			parameters.length()
		);
	}
	else
	{
		success = HttpSendRequestA(
			open_handle,
			content_type,
			-1L,
			nullptr,
			NULL
		);
	}

	if (!success)
	{
		DbgPrintf("HttpSendRequest failed: %lu\n", GetLastError());
		return {};
	}

    char buffer[1024]{};
    unsigned long bytes_read{};
    std::string response{};
    do
    {
        response.append(buffer);
		if (!InternetReadFile(open_handle, buffer, 1024, &bytes_read))
		{
			DbgPrintf("InternetReadFile error: %lu", GetLastError());
			return {};
		}
    } while (bytes_read > 0U);

    InternetCloseHandle(open_handle);
    return response;
}

// parses the given field until a given separator is found
// returns the parsed fields' value through the second parameter
// the default separator is comma
void TGBot::parseFirstOfField(const std::string_view& str, std::string& output, std::string_view&& token)
{
	char separator = ',';
	const auto pos = str.find(token);
	if (pos == std::string::npos)
	{
		if (m_bThrow) throw std::exception("couldn't parse text field from json");
		else return;
	}
	size_t start = pos + token.length();

	// check if its a string field
	if (str[start] == '"')
	{
		start++;
		separator = '"';
	}

	for (size_t i = start; i < str.length(); ++i)
	{
		if (str[i] == separator) break;
		output += str[i];
	}
}

// does what the name says
// it parses a singular message json object from the array of
// message objects from the response into a message struct containing:
// update_id, chat_id, date, username, and text
// can be easily modified to parse more fields
TGBot::tg_message TGBot::parseMessageResponseObject(const std::string_view& str)
{
	tg_message message{};
	std::string update_id, chat_id, date;

	constexpr std::string_view update_id_token = R"("update_id":)";
	parseFirstOfField(str, update_id, update_id_token.data());
    message.update_id = _strtoi64(update_id.c_str(), nullptr, 10);

	constexpr std::string_view chat_id_token = R"("chat":{"id":)";
	parseFirstOfField(str, chat_id, chat_id_token.data());
	message.chat_id = _strtoi64(chat_id.c_str(), nullptr, 10);

	constexpr std::string_view username_token = R"("username":)";
	parseFirstOfField(str, message.username, username_token.data());

	constexpr std::string_view date_token = R"("date":)";
	parseFirstOfField(str, date, date_token.data());
	message.date = _strtoi64(date.c_str(), nullptr, 10);

	constexpr std::string_view text_token = R"("text":)";
	parseFirstOfField(str, message.text, text_token.data());

    return message;
}

std::vector<TGBot::tg_message> TGBot::getUpdates()
{
	const std::string str = makeGetUpdateRequest();

	std::vector<TGBot::tg_message> messages;

	// unreliable in the sense that if they add even one character it breaks the parsing
	if (str != R"({"ok":true,"result":[]})" && !str.empty())
	{
		unsigned int level{};
		for (size_t start_index{}, i = 0; i < str.length(); ++i)
		{
			// searches for the start index of the second pair of brackets ({})
			// it is unique to the telegram response to always have the second pair of brackets as a message json body inside of a json object array
			// {
			//    ...
			//    "messages": [
			//    {message1}, {message2}, ...
			//    ],
			//    ...
			// }
			// therefore there is no need for a general purpose json parser
			if (str[i] == '{')
			{
				if (level == 1) start_index = i;
				++level;
			}

			else if(str[i] == '}') 
			{
				--level;
				if (level == 1) 
				{
					// we now have the starting and ending indexes of a single message object
					// parse it and put it in a messages vector
					messages.emplace_back(parseMessageResponseObject(str.substr(start_index, i-start_index)));
				}
			}

		}
		if (level != 0)
		{
			parse_error = true;
			DbgPrint("error while parsing json: the json is invalid");
			return {};
		}
		parse_error = false;
		// we set the last update id, so we can always call the getUpdates method of the api
		// with a bigger update id offset, so that it clears the messages
		// there is no other way of deleting the messages serverside, other than them deleting themselves after 24 hours
		lastUpdateId  = messages[messages.size()-1].update_id;
		clearMessageQueue();
	}
	return messages;
}

// opens a connection to https://www.api.telegram.org/
// and stores the handle as a member variable
bool TGBot::connectToBot()
{
	m_hOpen= InternetOpenA(
		DEFAULT_USERAGENT,
		INTERNET_OPEN_TYPE_PRECONFIG,
		nullptr, 
		nullptr, 
		NULL
	);

    if (m_hOpen == nullptr) {
		InternetCloseHandle(m_hConnect);
		DbgPrint("InternetOpen failed\n");
	    return false;
    }

	// HTTPS is required by telegram
	// https://core.telegram.org/bots/api#making-requests
	// "All queries to the Telegram Bot API must be served over HTTPS"
	m_hConnect = InternetConnectA(
		m_hOpen, 
		apiAddress.c_str(), 
		INTERNET_DEFAULT_HTTPS_PORT, 
		nullptr, 
		nullptr, 
		INTERNET_SERVICE_HTTP, 
		NULL, 
		NULL
	);
	
	if (m_hConnect == nullptr)
	{
		DbgPrintf("InternetOpenUrlA error: %lu", GetLastError());
		if (m_bThrow) throw std::exception("couldn't connect to telegram servers");
		return false;
	}

	// we keep the connection open to avoid the overhead
	// of opening a new connection for every request
	return true;
}

// closes all internet handles and throws on error if allowed
void TGBot::closeAllConnections() const
{
	BOOL success{true};
	success &= InternetCloseHandle(m_hConnect);
	success &= InternetCloseHandle(m_hOpen);
	if (!success && m_bThrow)
	{
		throw std::exception(("couldn't close HINTERNET connection handle in TGBot.cpp at line: " + std::to_string(__LINE__)).c_str());
	}
}

// sends a message through the bot to the chat_id specified
// unicode is not yet supported
bool TGBot::sendMessage(const uint64_t& chat_id, const std::string& msg)
{
	const auto response = makeQuery("sendMessage", "POST", FORM_URL_ENCODED, "text=" + msg + "&chat_id=" + std::to_string(chat_id));
	if (response.find("\"ok\":true") == std::string::npos)
	{
		DbgPrint("error while sending message");
		return false;
	}
	return true;
}

void TGBot::clearMessageQueue()
{
	makeQuery("getUpdates", "GET", FORM_URL_ENCODED, "offset=" + std::to_string(lastUpdateId+1));
}

std::string TGBot::makeGetUpdateRequest()
{
	return makeQuery("getUpdates", "GET", FORM_URL_ENCODED);
}