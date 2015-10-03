#include "tcpscanner.h"
#include <iostream>

using namespace std;

void TcpScanner::Scan(Services* services)
{
	for (auto service : *services)
	{
		initSocket(service);
	}

	sleep(timeout);

	for (auto service : *services)
	{
		pollSocket(service);
	}
}

void TcpScanner::initSocket(Service* service)
{
	// set-up address to probe

	struct sockaddr_in addr;
	addr.sin_port = htons(service->port);
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, service->address, &addr.sin_addr.s_addr);

	// create socket

	auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	auto data = new ActiveTcpScanData();
	service->data = data;
	data->socket = sock;
	data->fdsets = new vector<fd_set*>();

	// set it to non-blocking

	u_long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);

	// allocate file descriptor sets

	auto rdset = new fd_set();
	FD_ZERO(rdset);
	FD_SET(sock, rdset);

	data->fdsets->push_back(rdset); // 0 -> read

	auto wrset = new fd_set();
	FD_ZERO(wrset);
	FD_SET(sock, wrset);

	data->fdsets->push_back(wrset); // 1 -> write

	auto exset = new fd_set();
	FD_ZERO(exset);
	FD_SET(sock, exset);

	data->fdsets->push_back(exset); // 2 -> error

	// start non-blocking connection process

	connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
}

void TcpScanner::pollSocket(Service* service)
{
	TIMEVAL tv = { 0, 0 };
	auto data = reinterpret_cast<ActiveTcpScanData*>(service->data);

	// check if socket is writable, which basically means the connection was successful

	// for some reason, Linux requires the first parameter to be counterintuitively socket+1, while Windows doesn't
	// time spent searching for this error: ~1.5 hours

	select(
		data->socket
#if Linux
			+ 1
#endif
		, data->fdsets->at(0), data->fdsets->at(1), data->fdsets->at(2), &tv
	);

	// check if the writable flag is set

	auto isOpen = FD_ISSET(data->socket, data->fdsets->at(1));

#if Linux
	if (isOpen)
	{
		// yet again Linux decided to troll me. all select() requests will become "writable", and you have
		// to check if there was an error or not, to actually determine if the connect() was successful

		int serr;
		socklen_t slen = sizeof(serr);
		getsockopt(data->socket, SOL_SOCKET, SO_ERROR, &serr, &slen);
		isOpen = serr == 0;
	}
#endif

	service->alive = isOpen == 1;

	// clean-up

	closesocket(data->socket);
}