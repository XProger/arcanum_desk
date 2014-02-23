#include "mtp.h"
#include "main.h"
#include "mini-gmp.h"
#include "utils.h"

int RSA_MOD[] = {
	0x3e0250c1,0x79db702f,0x64d0de85,0xcffe9c75,0xe628f30a,0xf4da419a,0x531bf0d6,0xf9a63581,
	0x2a8b8f1f,0x97bac90e,0x2e35ce20,0x68c5f6fc,0x4b42fc0f,0x498634d6,0x4b0bde02,0x4e9fd4d6,
	0xe3300258,0x5cd997ae,0x2b44198b,0xd8100a3c,0xec3f63f5,0x6a92d6ed,0x0dab6d7f,0x7f457ddb,
	0x841ba89e,0xffd6fc65,0x4011edfe,0xc091df11,0xafedca59,0x6c5f6297,0x47c7ec96,0x34695525,
	0x861d78ef,0x11f0346b,0x35d8e4fc,0x6e1990a0,0x440e5f9a,0xb67eaf49,0x07b9dd97,0x5fca9464,
	0x304a1081,0x76d26d5b,0x462c7265,0xf65d0eb6,0xb216fb80,0xf27e6010,0x602e6517,0x5f256c23,
	0x5f31286a,0x67a98340,0x4b21d791,0xf41d4cf6,0x94b10dfd,0x2a6ab24f,0x321b0357,0xd14ae6ee,
	0x88a68b5a,0x4ae7cd85,0x0f92fc5b,0xba59bf6a,0x6350755c,0x0f13e773,0x92da4290,0x1f257921,
};

#define RSA_EXP 0x00010001

int MTP::time_delta = 0;

MTP::MTP(const char *ip, int port) : last_message_id(0), auth_key(0), auth_key_id(0) {	
	transport = new Transport(this);
	transport->connect(ip, port);
}

MTP::~MTP() {
	delete transport;
	delete auth_key;
}

void MTP::onConnect() {
	LOG("onConnect\n");
	auth();
}

void MTP::onDisconnect() {
	LOG("onDisonnect\n");
}

void MTP::onReceive(const char *data, int length) {
	LOG("onReceive: %d bytes\n", length);

	if (length < 20) {
		LOG("invalid packet\n");
		return;
	} 

	TL::Object *obj = new TL::Object(&data[20]);
	switch (obj->id) {
		case TL_resPQ :
			resPQ(obj);
			break;
		case TL_server_DH_params_fail	:
		case TL_dh_gen_fail				:
		case TL_dh_gen_retry			:
			LOG("fail! :(\n");
			transport->disconnect();
			break;
		case TL_server_DH_params_ok :
			LOG("TL_server_DH_params_ok!\n");
			server_DH_params_ok(obj);
			break;
		case TL_dh_gen_ok :
			LOG("TL_dh_gen_ok!\n");
			dh_gen_ok(obj);
			break;
		default :
			LOG("object not handled: #%08x", obj->id);
	}	
	delete obj;
}

void MTP::sendMessage(const char *data, int length) {
// calculate message_id
	LOG("send msg: %d bytes\n", length);

	__int64	message_id = (getUnixTime() << 32) + 4;
	if (message_id < last_message_id)
		message_id = last_message_id + 4;
	last_message_id = message_id;

	if (!auth_key) { // raw message		
	//	LOG_dump(data, length);

		transport->begin((8 + 8 + 4) + length);
		transport->send((char*)&auth_key_id, 8);
		transport->send((char*)&message_id, 8);
		transport->send((char*)&length, 4);		
		transport->send(data, length);
		transport->end();
	} else {				// encrypted message
		int padding = 16 - length % 16;	// ???	
		transport->begin((8 + 16 + 32) + length + padding);
/*
		send(auth_key_id, 8)
		send(msg_key, 16)
		// header
		aes->setKey(msg_key)
		header.setLong(salt)
		header.setLong(session_id)
		header.setLong(message_id)
		header.setInt(seq_no)
		header.setInt(length)
		aes->crypt(header, 32)
		send(header, 32)
		aes->crypt(data, length)
		send(data, length + padding)
*/
		transport->end();
	}
//	addMessage(message_id, data, length)
}

void MTP::auth() {
	TL::Int128 nonce;
	nonce.random();

	TL::begin();
	TL::_object(TL_req_pq);
	TL::_int128(nonce);
	sendMessage(TL::buf, TL::pos);
	TL::end();
}


void MTP::resPQ(TL::Object *obj) {
	TL::Int128 nonce(obj->_int128(0));
	TL::Int128 server_nonce(obj->_int128(1));
	new_nonce.random();

	__int64 fp = obj->_vector(3)->_long(0);

	TL::String *str = (TL::String*)(obj->params[2].value);

	mpz_t pq, p, q;
	mpz_init(pq);
	mpz_init_set_ui(p, 1);
	mpz_init(q);
	mpz_import(pq, str->length(), 1, 1, 0, 0, str->ptr());	

	LOG("pq: "); mpz_out_str(stdout, 10, pq); LOG("\n");

	brent(pq, p);
	mpz_divexact(q, pq, p);

	if (mpz_cmp(p, q) > 0)
		mpz_swap(p, q);	

	LOG("p: "); mpz_out_str(stdout, 10, p); LOG("\n");
	LOG("q: "); mpz_out_str(stdout, 10, q);	LOG("\n");

// generate encrypted_data
	TL::begin(20);
	TL::_object(TL_p_q_inner_data);
	TL::_bytes(pq);
	TL::_bytes(p);
	TL::_bytes(q);
	TL::_int128(nonce);
	TL::_int128(server_nonce);
	TL::_int256(new_nonce);	

	SHA1(&TL::buf[20], TL::pos - 20, TL::buf);
	
	int encrypted_data_size;
	char *encrypted_data = RSA(TL::buf, 255, &RSA_MOD, RSA_EXP, &encrypted_data_size);
	TL::end();

// send req_DH_params
	TL::begin();
	TL::_object(TL_req_DH_params);
	TL::_int128(nonce);
	TL::_int128(server_nonce);
	TL::_bytes(p);
	TL::_bytes(q);
	TL::_long(fp);	
	TL::_bytes(encrypted_data, encrypted_data_size);
	sendMessage(TL::buf, TL::pos);
	TL::end();

	delete encrypted_data;
	mpz_clear(pq);
	mpz_clear(p);
	mpz_clear(q);	
}

void MTP::server_DH_params_ok(TL::Object *obj) {
	
	TL::Int128 nonce(obj->_int128(0));
	TL::Int128 server_nonce(obj->_int128(1));
	
	TL::String *str = obj->_string(2);

	int encrypted_answer_size = str->length();
	char *encrypted_answer = str->ptr();

	aes.prepare(&server_nonce, &new_nonce, false);
	aes.IGE((uint32*)encrypted_answer, encrypted_answer_size);
	
	TL::Object *server_DH_inner_data = new TL::Object(&encrypted_answer[20]);

	mpz_t g, dh_prime, g_a;	
// g
	mpz_init_set_ui(g, server_DH_inner_data->_int(2));
// dh_prime
	str = server_DH_inner_data->_string(3);
	mpz_init(dh_prime);
	mpz_import(dh_prime, str->length(), 1, 1, 0, 0, str->ptr());	
// g_a
	str = server_DH_inner_data->_string(4);
	mpz_init(g_a);
	mpz_import(g_a, str->length(), 1, 1, 0, 0, str->ptr());
	
	time_delta = server_DH_inner_data->_int(5) - getUnixTime();
	
	delete server_DH_inner_data;
	
	LOG("time_delta: %d\n", time_delta);
	
	char b_data[256];
	for (int i = 0; i < sizeof(b_data); i++)
		b_data[i] = rand() & 0xff;
	mpz_t b, g_b;
	mpz_init(b);
	mpz_import(b, sizeof(b_data), 1, 1, 0, 0, &b_data);	

	mpz_init(g_b);
	mpz_powm(g_b, g, b, dh_prime);

// generate encrypted_data (client_DH_inner_data)
	TL::begin(20);
	TL::_object(TL_client_DH_inner_data);
	TL::_int128(nonce);
	TL::_int128(server_nonce);
	TL::_long(0);
	TL::_bytes(g_b);
	SHA1(&TL::buf[20], TL::pos - 20, TL::buf);
	int encrypted_data_size = (TL::pos + 15) / 16 * 16;
	char *encrypted_data = new char[encrypted_data_size];
	memcpy(encrypted_data, TL::buf, encrypted_data_size);
	TL::end();

	aes.prepare(&server_nonce, &new_nonce, true);
	aes.IGE((uint32*)encrypted_data, encrypted_data_size);
	
// send set_client_DH_params
	TL::begin();
	TL::_object(TL_set_client_DH_params);
	TL::_int128(nonce);
	TL::_int128(server_nonce);
	TL::_bytes(encrypted_data, encrypted_data_size);
	sendMessage(TL::buf, TL::pos);
	TL::end();

	delete encrypted_data;

// generate auth_key
	mpz_powm(g, g_a, b, dh_prime);

	int auth_key_size;
	auth_key = mpz_bytes(g, &auth_key_size);
	
	uint32 auth_key_hash[5];
	SHA1(auth_key, auth_key_size, &auth_key_hash);
	memcpy(&auth_key_id, auth_key_hash, 8);

// generate server salt
	int64 *s1 = (int64*)&new_nonce;
	int64 *s2 = (int64*)&server_nonce;
	server_salt = *s1 ^ *s2;

	mpz_clear(g);
	mpz_clear(g_a);
	mpz_clear(g_b);
	mpz_clear(dh_prime);
	mpz_clear(b);	
}

void MTP::dh_gen_ok(TL::Object *obj) {	
	//
}