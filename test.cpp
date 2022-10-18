// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#ifdef _DEBUG

#include "TGBot.hpp"

int main()
{
	// testing message queries
	TGBot bot("5088914784:AAG5t6oR17RO4UVp1qpUi_clnFF2W-z-iic");
	auto messages = bot.getUpdates();
	for (const auto& [update_id, chat_id, date, username, text] : messages)
	{
		printf("%s wrote %s chatid: %llu\n", username.c_str(), text.c_str(), chat_id);
		bot.sendMessage(chat_id, text);
	}
	return 0;
}

#endif