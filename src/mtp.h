#ifndef MTP_H
#define MTP_H

#include "transport.h"
#include "tl.h"

struct MTP : public Transport::ICallback {
	static int time_delta;

	Transport	*transport;
	TL::Int256	new_nonce;
	AES			aes;
	uint64		last_message_id;
	char*		auth_key;
	int64		auth_key_id;
	int64		server_salt;

	MTP(const char *ip, int port);
	~MTP();
	virtual void onConnect();
	virtual void onDisconnect();
	virtual void onReceive(const char *data, int length);
	void sendMessage(const char *data, int length);
	void auth();
	void resPQ(TL::Object *obj);
	void server_DH_params_ok(TL::Object *obj);
	void dh_gen_ok(TL::Object *obj);
};

#endif