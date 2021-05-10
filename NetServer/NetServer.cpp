// NetClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <net_message.h>
#include <net_server.h>

enum class MyMsgType : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage
};

class MyServer : public server_interface<MyMsgType>
{
public:
	MyServer(uint16_t port)
		: server_interface<MyMsgType>(port)
	{}

protected:

	bool OnConnect(std::shared_ptr<connection<MyMsgType>>& client) override
	{
		message<MyMsgType> msg;
		msg.header.id = MyMsgType::ServerAccept;
		SendToClient(client, msg);
		return true;		
	}

	void OnDisconnect(std::shared_ptr<connection<MyMsgType>>& client) override
	{
		spdlog::info("client:{} disconnect", client->GetID());	
	}

	void OnMessage(std::shared_ptr<connection<MyMsgType>> client, message<MyMsgType>& msg) override
	{
		switch (msg.header.id)
		{
			case MyMsgType::ServerAccept: break;
			case MyMsgType::ServerDeny: break;
			case MyMsgType::ServerPing: 
				spdlog::info("[{}] Ping", client->GetID());
				client->Send(msg);
				break;
			case MyMsgType::MessageAll:
				{
				message<MyMsgType> reply;
				reply.header.id = MyMsgType::ServerMessage;
				SendToAllClients(reply, client);
				spdlog::info("client:{} send to all", client->GetID());
				}
				break;
			case MyMsgType::ServerMessage: break;
			default: ;
		}
	}
};

int main()
{
	spdlog::set_level(spdlog::level::debug);
	MyServer server(9981);
	server.Start();

	while(1)
	{
		server.update(-1, true);
	}
	return 0;
}

