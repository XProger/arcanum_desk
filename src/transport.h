#ifndef TRANSPORT_H
#define TRANSPORT_H

#ifdef WIN32
	#include <winsock.h>
#endif

struct Transport {
	struct ICallback {
		virtual void onConnect() {};
		virtual void onDisconnect() {};
		virtual void onReceive(const char* data, int length) {};
	} *callback;

	SOCKET sock;
	bool connected;

	Transport(ICallback *callback);
	~Transport();
	void connect(const char *ip, int port);
	void disconnect();
	void send(const char *data, int length);
	void begin(int length);
	void end();
};

#endif