#include "transport.h"
#include "main.h"

int _stdcall recvLoop(const Transport *transport) {	
	char *buf = new char[TL_BUF_SIZE];
	int readed, len, count = 0;
	LOG("start recvLoop\n");
	while ((readed = recv(transport->sock, &buf[count], TL_BUF_SIZE - count, 0)) > 0) {
		count += readed;
		while (true) {
			int pos = 1;
			len = buf[0];
			if (len == 0x7f) {
				len = (*((int*)&buf[0]) & 0xFFFFFF00) >> 8;
				pos = 4;
			}
			len *= 4;

			if (count < pos + len)
				break;

			transport->callback->onReceive(&buf[pos], len);

			count -= pos + len;
			if (count > 0)
				memmove(&buf[0], &buf[pos + len], count);
		}
	}
	LOG("end recvLoop %d\n", readed);
	delete buf;
	return 0;
}

Transport::Transport(ICallback *callback) : callback(callback), connected(false) {
}

Transport::~Transport() {
	disconnect();
}

void Transport::connect(const char *ip, int port) {
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family   = AF_INET;
	sa.sin_port		= htons(port);
	sa.sin_addr.S_un.S_addr = inet_addr(ip);

	if (::connect(sock, (sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR) {
		LOG("error: connect to MTP server\n");
		return;
	};

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&recvLoop, this, 0, 0);
/*
	int SOCK_TIMEOUT = 30 * 1000;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&SOCK_TIMEOUT, sizeof(SOCK_TIMEOUT));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&SOCK_TIMEOUT, sizeof(SOCK_TIMEOUT));
*/
	int SOCK_SIZE = TL_BUF_SIZE;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&SOCK_SIZE, sizeof(SOCK_SIZE));
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&SOCK_SIZE, sizeof(SOCK_SIZE));

	int liteFlag = 0xef;
	send((char*)&liteFlag, 1);

	callback->onConnect();
}

void Transport::disconnect() {
	if (sock == INVALID_SOCKET) return;
    shutdown(sock, 2);
	closesocket(sock);
	sock = INVALID_SOCKET;
	callback->onDisconnect();
}

void Transport::send(const char *data, int length) {
	if (sock == INVALID_SOCKET) return;

	while (length > 0) {
		int count = ::send(sock, data, length, 0);
		if (count == SOCKET_ERROR) {
			LOG("...error %d\n", WSAGetLastError());
			disconnect();
			return;
		}
		length -= count;
		data += count;
	}
}

void Transport::begin(int length) {
	int len = length / 4;
	if (len >= 0x7f)
		len = (len << 8) | 0x7f;
	send((char*)&len, len < 0x7f ? 1 : 4);
}

void Transport::end() {
	// send CRC32 for non lite transport
}