#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"

template <typename T>
class connection : public std::enable_shared_from_this<connection<T>>
{
public:
	connection()
	{
		
	}

	virtual ~connection()
	{
		
	}

	bool Connect(const asio::ip::tcp::endpoint& ep);
	bool Disconnect();
	bool IsConnected() const;

	bool Send(const message<T>& msg);

protected:
	asio::ip::tcp::socket socket_;
	asio::io_context& io_context_;	

	tsqueue<message<T>> queue_out_;
	tsqueue<owned_message<T>>& queue_in_;
};
