#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

template <typename T>
class server_interface
{
public:
	server_interface(uint16_t port)
		: acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{
		
	}

	virtual ~server_interface()
	{
		Stop();	
	}

	bool Start()
	{
		try
		{
			WaitForClient();
			io_thread_ = std::thread([this](){io_context_.run();});
		}
		catch (std::exception& e)
		{
			spdlog::error("start exception:{}", e.what());
			return false;
		}	

		spdlog::debug("server started");
		return true;
	}

	void Stop()
	{
		io_context_.stop();
		if(io_thread_.joinable())  io_thread_.join();
		spdlog::debug("server stoped");
	}

	void WaitForClient()
	{
		acceptor_.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket)
		{
			if(!ec)
			{
				spdlog::debug("new connection from: {}:{}", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());

				 std::shared_ptr<connection<T>> newconn =
				 	std::make_shared<connection<T>>(connection<T>::owner::server,
				 		io_context_, std::move(socket), message_queue_in_);
				
				 if(OnConnect(newconn))
				 {
				 	connections_.push_back(std::move(newconn));	
				 	connections_.back()->ConnectToClient(client_id_base_++);
				 	spdlog::debug("new connect accepted {}",connections_.back()->GetID());
				 }
				 else
				 {
				 	spdlog::debug("connect from {}:{} reject", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());
				 }
			}
			else
			{
				spdlog::error("accept error: {}", ec.message());
			}
			WaitForClient();
		});	
	}

	void SendToClient(std::shared_ptr<connection<T>>& client, const message<T>& msg)
	{
		if(client && client->IsConnected())
		{
			client->Send(msg);	
		}
		else
		{
			OnDisconnect(client);	
			client.reset();

			connections_.erase(std::remove(connections_.begin(), connections_.end(), client), connections_.end());
		}
	}

	void SendToAllClients(const message<T>& msg, std::shared_ptr<connection<T>>& ignore_client = nullptr)
	{
		bool disconnect_found = false;
		for(auto& client : connections_)
		{
			if(client && client->IsConnected())
			{
				if( client != ignore_client)
				{
					client->Send(msg);
				}
			}
			else
			{
				disconnect_found = true;
				OnDisconnect(client);
				client.reset();
			}
		}

		if(disconnect_found)
		{
			connections_.erase(
				std::remove(connections_.begin(), connections_.end(), nullptr), connections_.end());
		}
	}

	void update(size_t max_messages = -1)
	{
		size_t message_cnt = 0;	
		while(message_cnt < max_messages && !message_queue_in_.empty())
		{
			auto msg = message_queue_in_.pop_front();
			OnMessage(msg.remote, msg.msg);
			message_cnt++;
		}
	}

protected:
	virtual bool OnConnect(std::shared_ptr<connection<T>>& client)
	{
		return false;	
	}

	virtual void OnDisconnect(std::shared_ptr<connection<T>>& client)
	{
		
	}

	virtual  void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
	{
		
	}

	std::deque<std::shared_ptr<connection<T>>> connections_;
	tsqueue<owned_message<T>> message_queue_in_;
	asio::io_context io_context_;
	std::thread	io_thread_;

	asio::ip::tcp::acceptor acceptor_;
	uint32_t client_id_base_ = 1000;
};

