#include "includes.hpp"
#include "TGBot.hpp"

int main()
{
	TGBot tgbot("5088914784:AAG5t6oR17RO4UVp1qpUi_clnFF2W-z-iic");
	//tgbot.sendMessage(1518862207, "xddd");
	while (true)
	{
		//printf("%s\n", tgbot.makeUrlQuery(tgbot.urlQueryString + "getUpdates").c_str());
		for (const auto& [update_id, chat_id, date, username, text] : tgbot.getUpdates())
		{
			printf("update_id: %llu\nchat_id: %llu\ndate: %llu\nusername: %s\nmessage: %s\n\n", update_id, chat_id, date, username.c_str(), text.c_str());
		}
		Sleep(2000);
		system("cls");
	}
}