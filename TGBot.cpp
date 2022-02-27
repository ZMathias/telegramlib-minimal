#include "TGBot.hpp"

std::string TGBot::makeUrlQuery(const std::string& queryUrl)
{
	const HINTERNET handle = InternetOpenA("HTTPS", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (handle == nullptr)
    {
        return {};
    }
	const HINTERNET file_handle = InternetOpenUrlA(handle, queryUrl.c_str(), nullptr, 0, INTERNET_FLAG_RESYNCHRONIZE, 0);

	if (file_handle == nullptr)
	{
#ifdef _DEBUG 
		printf("InternetOpenUrlA error: %lu", GetLastError());
#endif

		return {};
	}

    char buffer[1000]{};
    unsigned long bytesRead{};
    std::string s{};
    do
    {
        s.append(buffer);
		if (!InternetReadFile(file_handle, buffer, 1000, &bytesRead))
		{
#ifdef _DEBUG
			internet_error = true;
			printf("InternetReadFile error: %lu", GetLastError());
			return {};
#endif
		}
		internet_error = false;
    } while (bytesRead > 0U);

    InternetCloseHandle(handle);
    return s;
}

void TGBot::parseUntil(const std::string_view& str, std::string& output, std::string_view&& end_token, const size_t& offset, const char& separator)
{
	const auto pos = str.find(end_token);
	for (size_t i = pos + offset; i < str.length(); ++i)
	{
		if (str[i] == separator) break;
		output += str[i];
	}
}

TGBot::tg_message TGBot::parseMessageObject(const std::string_view& str)
{
	std::string slave;
	tg_message message;
	std::string update_id, chat_id, date;

    parseUntil(str, update_id, "\"update_id\":", std::strlen("\"update_id\":"));
    message.update_id = _strtoi64(update_id.c_str(), nullptr, 10);

	parseUntil(str, chat_id, R"("chat":{"id":)", std::strlen(R"("chat":{"id":)"), ',');
	message.chat_id = _strtoi64(chat_id.c_str(), nullptr, 10);

	parseUntil(str, message.username, "\"username\":", std::strlen("\"username\":") + 1, '\"');

	parseUntil(str, date, "\"date\":", std::strlen("\"date\":"));
	message.date = _strtoi64(date.c_str(), nullptr, 10);

	parseUntil(str, message.text, R"("text":")", std::strlen("\"text\":") + 1, '"');
    return message;
}

std::vector<TGBot::tg_message> TGBot::getUpdates()
{
	const std::string str(makeUrlQuery(urlQueryString + "getUpdates"));
	std::vector<TGBot::tg_message> messages;
	if (str != R"({"ok":true,"result":[]})" && !str.empty())
	{
		unsigned int level{};
		for (size_t start_index{}, i = 0; i < str.length(); ++i)
		{
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
					messages.push_back(parseMessageObject(str.substr(start_index, i-start_index)));
				}
			}

		}
		if (level != 0)
		{
			parse_error = true;
#ifdef _DEBUG
			printf("error while parsing json: the json is invalid");
#endif
			return {};
		}
		parse_error = false;
		last_update_id  = messages[messages.size()-1].update_id;
		clearMessageQueue();
	}
	return messages;
}

void TGBot::sendMessage(const uint64_t& chat_id, const std::string& msg) const
{
    makeUrlQuery(urlQueryString + "sendMessage?chat_id=" + std::to_string(chat_id) + "&text=" + msg);
}

void TGBot::clearMessageQueue() const
{
	makeUrlQuery(urlQueryString + "getUpdates?offset=" + std::to_string(last_update_id+1));
}

