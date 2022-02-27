#include "includes.hpp"
#include "TGBot.hpp"

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR     lpCmdLine,
				   int       nShowCmd)
{
	TGBot tgbot("5088914784:AAG5t6oR17RO4UVp1qpUi_clnFF2W-z-iic");
	while (true)
	{
		for (const auto& [update_id, chat_id, date, username, text] : tgbot.getUpdates())
		{
			if (text == "/status") tgbot.sendMessage(chat_id, "online and polling");
			else if (text == "/kill") return 0;
			else tgbot.sendMessage(chat_id, text);
		}
		Sleep(2500);
	}
}