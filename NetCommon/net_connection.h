#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_validation.h"

template <typename T>
class connection : public std::enable_shared_from_this<connection<T>>
{
public:
	enum class owner
	{
		server,
		client
	};
	connection(owner type, asio::io_context& io_context, asio::ip::tcp::socket s, tsqueue<owned_message<T>>& in_message_queue)
		: io_context_(io_context),
		socket_(std::move(s)),
		queue_in_(in_message_queue)
	{
		type_ = type;	

		tmp_validation_ = 0;
		tmp_validation2_ = validate_.get_init();
	}

	virtual ~connection()
	{
		
	}

	uint32_t GetID() const
	{
		return id_;
	}

	bool ConnectToClient(uint32_t uid = 0)
	{
		if(type_ == owner::server)
		{
			if(socket_.is_open())
			{
				id_ = uid;
				//ReadHeader();

				WriteValidation();
				return true;
			}
			return false;
		}
		return false;
	}

	bool ConnectToServer(const asio::ip::tcp::resolver::results_type& ep)
	{
		if(type_ == owner::client)
		{
			asio::async_connect(
				socket_, 
				ep, 
				[this](asio::error_code ec, asio::ip::tcp::endpoint ep)
				{
					if(!ec)
					{
						ReadValidation();
					}
					else
					{
						spdlog::error("connect failed:{}", ec.message());
					}
				});
			return true;
		}
		return false;
	}

	bool Disconnect()
	{
		if(IsConnected())
		{
			asio::post(io_context_, [this]()
				{
					socket_.close();
				});
		}
		return true;
	}
	bool IsConnected() const
	{
		return socket_.is_open();
	}

	bool Send(const message<T>& msg)
	{
		asio::post(io_context_, [this, msg]()
		{
			bool is_writing = !queue_out_.empty();
			queue_out_.push_back(msg);	
			if(!is_writing)
			{
				WriteHeader();
			}
		});
		return true;
	}

protected:
	asio::ip::tcp::socket socket_;
	asio::io_context& io_context_;	

	owner type_ = owner::server;
	tsqueue<message<T>> queue_out_;
	tsqueue<owned_message<T>>& queue_in_;
	message<T> tmp_message_;

	uint32_t id_{0};

private:
	uint64_t tmp_validation_;
	uint64_t tmp_validation2_;
	validation validate_;

	void ReadValidation()
	{
		asio::async_read(socket_, asio::buffer(&tmp_validation_, sizeof(uint64_t)),
			[this](asio::error_code ec, std::size_t length)
			{
				if(!ec)
				{
					if(type_ == owner::server)
					{
						validate_.set_other(tmp_validation_);
						if(!validate_.validate())
						{
							spdlog::error("validate error");
							socket_.close();	
						}
						else
						{
							ReadHeader();	
						}
					}
					else
					{
						tmp_validation2_ = validate_.hash_it(tmp_validation_);		
						WriteValidation();
					}
				}
				else
				{
					spdlog::error("[{}] recv validation failed:{}", id_, ec.message());
					socket_.close();
				}
			});	
	}

	void WriteValidation()
	{
		asio::async_write(socket_, asio::buffer(&tmp_validation2_, sizeof(uint64_t)),
			[this](asio::error_code ec, std::size_t length)
			{
				if(!ec)
				{
					if(type_ == owner::server)
					{
						ReadValidation();		
					}
					else
					{
						ReadHeader();	
					}
				}
				else
				{
					spdlog::error("[{}] write validation failed:{}", id_, ec.message());
					socket_.close();
				}
			});
	}

	void ReadHeader()
	{
		asio::async_read(socket_, asio::buffer(&tmp_message_.header, sizeof(message_header<T>)),
			[this](asio::error_code ec, std::size_t length)
			{
				if(!ec)
				{
					if(tmp_message_.header.size > 0)
					{
						tmp_message_.body.resize(tmp_message_.header.size);
						ReadBody();
					}
					else
					{
						AddToIncomingQueue();
					}
				}
				else
				{
					spdlog::error("[{}] recv header failed:{}", id_, ec.message());
					socket_.close();
				}
			});	
	}

	void ReadBody()
	{
		asio::async_read(socket_, asio::buffer(tmp_message_.body.data(), tmp_message_.body.size()), 
			[this](asio::error_code ec, std::size_t length)
			{
				if(!ec)
				{
					AddToIncomingQueue();
				}
				else
				{
					spdlog::error("[{}] recv body failed:{}", id_, ec.message());
					socket_.close();
				}
			});	
	}

	void WriteHeader()
	{
		asio::async_write(socket_, asio::buffer(&queue_out_.front().header, sizeof(message_header<T>)),
			[this](asio::error_code ec, std::size_t length)
			{
				if(!ec)
				{
					if(queue_out_.front().body.size() > 0)
					{
						WriteBody();	
					}
					else
					{
						queue_out_.pop_front();
						if(!queue_out_.empty())
						{
							WriteHeader();
						}
					}
				}
				else
				{
					spdlog::error("[{}] write header failed:{}", id_, ec.message());
					socket_.close();
				}
			});
	}

	void WriteBody()
	{
		asio::async_write(socket_, asio::buffer(queue_out_.front().body.data(), queue_out_.front().body.size()),
			[this](asio::error_code ec, std::size_t length)
			{
				if(!ec)
				{
					queue_out_.pop_front();
					if (!queue_out_.empty())
					{
						WriteHeader();
					}
				}
				else
				{
					spdlog::error("[{}] write header failed:{}", id_, ec.message());
					socket_.close();
				}
			});
	}

	void AddToIncomingQueue()
	{
		if(type_ == owner::server)
		{
			queue_in_.push_back({this->shared_from_this(), tmp_message_});
		}
		else
		{
			queue_in_.push_back({nullptr, tmp_message_});
		}

		ReadHeader();
	}
};
