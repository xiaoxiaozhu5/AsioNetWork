// NetClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <net_message.h>
#include <net_client.h>

enum class MyMsgType : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage
};

class MyClient : public client_interface<MyMsgType>
{
public:
	void PingServer()
	{
		message<MyMsgType> msg;
		msg.header.id = MyMsgType::ServerPing;

		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		msg << now;
		Send(msg);
	}

	void SendToAll()
	{
		message<MyMsgType> msg;
		msg.header.id = MyMsgType::MessageAll;
		Send(msg);
	}
};

int main()
{
	MyClient client;
	client.Connect("127.0.0.1", 9981);

	bool key[3] = {false, false, false};
	bool old_key[3] = {false, false, false};

	bool quit = false;
	while(!quit)
	{
		if(GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if(key[0] && !old_key[0])
		{
			client.PingServer();
		}
		if(key[1] && !old_key[1])
		{
			client.SendToAll();
		}
		if(key[2] && !old_key[2])
		{
			quit = true;
		}

		for(int i = 0; i < 3; i++)
		{
			old_key[i] = key[i];
		}
		
		
		if(client.IsConnected())
		{
			if(!client.Incoming().empty())
			{
				auto msg = client.Incoming().pop_front().msg;
				switch (msg.header.id)
				{
					case MyMsgType::ServerAccept: 
						spdlog::info("server accept me");
						break;
					case MyMsgType::ServerDeny: break;
					case MyMsgType::ServerPing:
						{
							std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
							std::chrono::system_clock::time_point then;
							msg >> then;
							spdlog::info("Ping: {}", std::chrono::duration<double>(now - then).count());
						}
						break;
					case MyMsgType::MessageAll: 
						break;
					case MyMsgType::ServerMessage: 
						spdlog::info("recv from server");
						break;
					default: ;
				}
			}
		}
		else
		{
			spdlog::info("server close");
			quit = true;
		}
	}
	return 0;
}

