#ifndef UTILS_H
#define UTILS_H

#include "mini-gmp.h"

#define LEFT_ROT(a,b) ((a<<b)|(a>>(32-b)))

struct HashEntry {
	int key;
	void *value;
	HashEntry(int key, void *value) : key(key), value(value) {}
};

struct HashMap {
	HashEntry **table;
	int size;

	HashMap(int size) : size(size) {
		table = new HashEntry*[size];
		for (int i = 0; i < size; i++)
			table[i] = 0;
	}

	~HashMap() {
		for (int i = 0; i < size; i++)
			delete table[i];
		delete table;
	}
 
	void* get(unsigned int key) {
		int hash = (key % size);
		while (table[hash] && table[hash]->key != key)
			hash = (hash + 1) % size;						// FIX! infinite loop		
		return table[hash] ? table[hash]->value : 0;
	}
 
	void put(unsigned int key, void *value) {
		int hash = (key % size);
		while (table[hash] && table[hash]->key != key)
			hash = (hash + 1) % size;
		if (table[hash])
			table[hash]->value = value;
		else
			table[hash] = new HashEntry(key, value);
	}
};

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef unsigned long long	uint64;

typedef char		int8;
typedef short		int16;
typedef int			int32;
typedef long long	int64;

extern uint32 swapUInt32(uint32 x);
extern int brent(const mpz_t N, mpz_t divisor);
extern int floyd(const mpz_t N, mpz_t divisor);
extern char* mpz_bytes(mpz_t value, int *outSize);

extern void SHA1(const char *data, int size, void *digest);
extern char* RSA(const char *data, int size, void *m, int e, int *outSize);

// based on axTLS code by Cameron Rich
#define AES_ROUNDS	14

struct AES {
	static uint8 sbox[256];
	static uint8 isbox[256];
	static uint8 Rcon[30];

	uint32	ks[(AES_ROUNDS + 1) * 8];
	char	key[32], iv[32];
	bool	encrypt;

	static void init();
	void setKey(const uint8 *key, bool encrypt);
	void prepare(void *server_nonce, void *new_nonce, bool encrypt);
	void IGE(uint32 *data, int size);
	void encryptBlock(uint32 *data);
	void decryptBlock(uint32 *data);
};

#endif