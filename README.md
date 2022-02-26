**THIS PROJECT IS IN IT'S EARLY STAGES
Don't expect a lot of functionality for the time being**

A very minimal, fast and compact Telegram Bot library.

# Features
* fast
* reliable
* based on C++20
* header-only
* only 23kb executable size
* low memory consumption
* dependency free (only relies on WinInet)

Rather than going down the webhook and long-polling rabbithole, this uses the "make a request every couple of seconds" approach.
Technically it's not as efficient as keeping the connection open until data is received, but a huge upside is, that it:
* cuts down in complexity and code
* makes the process a lot more reliable
* in some cases it's also faster


## Telegram API features

Already implemented:
- sending and receiving messages

Planned:
- sending and receiving files
- sending and receiving pictures
