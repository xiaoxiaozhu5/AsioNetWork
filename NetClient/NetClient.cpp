// NetClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <net_message.h>
#include <net_client.h>

enum class MyMsgType : uint32_t
{
	Fire,
	Move
};

class MyClient : public client_interface<MyMsgType>
{
public:
	bool Fire(float x, float y)
	{
		message<MyMsgType> msg;
		msg.header.id = MyMsgType::Fire;
		msg << x << y;
		Send(msg);
	}
};

int main()
{
	message<MyMsgType> msg;
	msg.header.id = MyMsgType::Fire;

	int a = 12;
	bool b = false;
	float c = 3.14159f;

	struct
	{
		float x;
		float y;
	}d[3];

	msg << a << b << c << d;

	a = 44;
	b = true;
	c = 0.618f;

	msg >> d >> c >> b >> a;

	MyClient client;
	client.Connect("example.com", 1080);
	client.Fire(1.0f, 2.0f);

	return 0;
}

