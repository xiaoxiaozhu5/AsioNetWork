#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

template <typename T>
class client_interface
{
public:
	client_interface() : socket_(io_context_)
	{
		
	}

	virtual ~client_interface()
	{
		Disconnect();
	}
	
	bool Connect(const std::string& host, const uint16_t& port)
	{
		asio::ip::tcp::resolver resolver(io_context_);
		asio::ip::tcp::resolver::results_type ep = resolver.resolve(host, std::to_string(port));

		connection_ = std::make_unique<connection<T>>(
			connection<T>::owner::client, 
			io_context_,
			asio::ip::tcp::socket(io_context_),
			queue_in_);

		connection_->ConnectToServer(ep);
		thread_ = std::thread([this](){io_context_.run();});
		return true;
	}

	void Disconnect()
	{
		if(IsConnected())
		{
			connection_->Disconnect();
		}

		io_context_.stop();
		if(thread_.joinable())
			thread_.join();

		connection_.release();
	}

	bool IsConnected()
	{
		if(connection_)
		{
			return connection_->IsConnected();
		}
		return false;
	}

	tsqueue<owned_message<T>>& Incoming()
	{
		return queue_in_;
	}

	bool Send(message<T>& msg)
	{
		return connection_->Send(msg);
	}

protected:
	asio::io_context io_context_;
	std::thread thread_;
	asio::ip::tcp::socket socket_;
	std::unique_ptr<connection<T>> connection_;
	
private:
	tsqueue<owned_message<T>> queue_in_;
};
