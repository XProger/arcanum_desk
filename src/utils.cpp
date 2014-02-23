#include "utils.h"
#include "main.h"

uint32 swapUInt32(uint32 x) {
	x = ((x << 8) & 0xFF00FF00) | ((x >> 8) & 0x00FF00FF); 
	return (x << 16) | (x >> 16);
}

void f(mpz_t result, mpz_t x, const mpz_t N) {
	mpz_set(result, x);
	mpz_mul(result, result, result);
	mpz_add_ui(result, result, 1);
	mpz_mod(result, result, N);
}

int brent(const mpz_t N, mpz_t divisor) {
	unsigned int iterations = 0;
	unsigned int x0 = 2;
	mpz_t power;		mpz_init_set_ui(power, 1);
	mpz_t lambda;		mpz_init_set_ui(lambda, 1);
	mpz_t tortoise;		mpz_init_set_ui(tortoise, x0);
	mpz_t hare;			mpz_init(hare); f(hare, tortoise, N);
	mpz_t diff;			mpz_init(diff);

	while (!mpz_cmp_ui(divisor, 1)) {

		if (!mpz_cmp(power, lambda)) {
			mpz_set(tortoise, hare);
			mpz_mul_ui(power, power, 2);
			mpz_set_ui(lambda, 0);
		}
		
		f(hare, hare, N);
		mpz_add_ui(lambda, lambda, 1);

		mpz_sub(diff, tortoise, hare);
		mpz_abs(diff, diff);

		if (!mpz_cmp_ui(diff, 0)) {
			x0++;
			iterations = 0;
			mpz_set_ui(power, 1);
			mpz_set_ui(lambda, 1);
			mpz_set_ui(tortoise, x0);
			mpz_set_ui(divisor,1);
			f(hare, tortoise, N);
			continue;
		}
		
		mpz_gcd(divisor,diff,N);
	}

	mpz_clear(diff);
	mpz_clear(power);
	mpz_clear(lambda);
	mpz_clear(tortoise);
	mpz_clear(hare);
	return !mpz_cmp(divisor, N) ? 0 : 1;
}


int floyd(const mpz_t N, mpz_t divisor) {
	mpz_t x;	mpz_init_set_ui(x, 2);
	mpz_t y;	mpz_init_set_ui(y, 2);

	while (!mpz_cmp_ui(divisor, 1)) {
		f(x, x, N); // x = f(x)
		f(y, y, N); // y = f(x)
		f(y, y, N); // y = f(x)

		mpz_t diff;		
		mpz_init(diff);
		mpz_sub(diff, x, y);
		mpz_abs(diff, diff);

		if (!mpz_cmp_ui(diff, 0)){
			mpz_clear(x);
			mpz_clear(y);
			return 0;
		}

		mpz_gcd(divisor, diff, N);
		mpz_clear(diff);
	}

	mpz_clear(x);
	mpz_clear(y);
	return 1;
}

char* mpz_bytes(mpz_t value, int *outSize) {
	return (char*)mpz_export(0, (size_t*)outSize, 1, 1, 0, 0, value);
}

void SHA1(const char *data, int size, void *digest) {
	unsigned int *h = (unsigned int*)digest;
	h[0] = 0x67452301;
	h[1] = 0xEFCDAB89;
	h[2] = 0x98BADCFE;
	h[3] = 0x10325476;
	h[4] = 0xC3D2E1F0;

	unsigned int w[80];
	int len = 0, k = 1;

	while (len != size || k) {
	// fill 512-bit buffer
		k = min(64, size - len);
		memcpy(&w[0], &data[len], k);
		len += k;

		if (len == size) {
			if ((k > 0 && k < 64) || (k == 0 && (size % 64) == 0))
				((unsigned char*)&w)[k++] = 0x80;
			memset(&((char*)&w)[k], 0, 64 - k);
			if (k < 57) {
			//	w[14] = 0
				w[15] = swapUInt32(size * 8);
				k = 0;
			}
		}


	// swap byte order to Big Endian
		for (int i = 0; i < 16; i++)
			w[i] = swapUInt32(w[i]);
	// sha1 magic
		for (int i = 16; i < 80; i++) {
			w[i] = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
			w[i] = (w[i] << 1) | (w[i] >> 31);
		}

	// sha1 magic of magic
		unsigned int a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f, g, t;
		for (int i = 0; i < 80; i++) {
			if (i > 59) {
				f = b ^ c ^ d;
				g = 0xCA62C1D6;
			} else
				if (i > 39) {
					f = (b & c) | (b & d) | (c & d);
					g = 0x8F1BBCDC;
				} else
					if (i > 19) {
						f = b ^ c ^ d;
						g = 0x6ED9EBA1;
					} else {
						f = d ^ (b & (c ^ d));
						g = 0x5A827999;
					}
			t = LEFT_ROT(a, 5) + f + e + g + w[i];
			e = d;
			d = c;
			c = LEFT_ROT(b, 30);
			b = a;
			a = t;
		}

		h[0] += a;
		h[1] += b;
		h[2] += c;
		h[3] += d;		
		h[4] += e;		
	}

// swap result byte order to Little Endian
	h[0] = swapUInt32(h[0]);
	h[1] = swapUInt32(h[1]);
	h[2] = swapUInt32(h[2]);
	h[3] = swapUInt32(h[3]);
	h[4] = swapUInt32(h[4]);
}

char* RSA(const char *data, int size, void *m, int e, int *outSize) {
	mpz_t exp, mod, d, r;	
	mpz_init_set_ui(exp, e);
	mpz_init(mod);
	mpz_import(mod, 256, 1, 1, 0, 0, m);
	
	mpz_init(d);
	mpz_init(r);	
	mpz_import(d, size, 1, 1, 0, 0, data);

	mpz_powm(r, d, exp, mod);
	char *res = mpz_bytes(r, outSize);

	mpz_clear(exp);
	mpz_clear(mod);
	mpz_clear(d);
	mpz_clear(r);

	return res;
}

// AES
#define rot1(x) (((x) << 24) | ((x) >> 8))
#define rot2(x) (((x) << 16) | ((x) >> 16))
#define rot3(x) (((x) <<  8) | ((x) >> 24))

#define mt  0x80808080
#define ml  0x7f7f7f7f
#define mh  0xfefefefe
#define mm  0x1b1b1b1b
#define mul2(x,t)	((t)=((x)&mt),((((x)+(x))&mh)^(((t)-((t)>>7))&mm)))

#define inv_mix_col(x,f2,f4,f8,f9) (\
			(f2)=mul2(x,f2), \
			(f4)=mul2(f2,f4), \
			(f8)=mul2(f4,f8), \
			(f9)=(x)^(f8), \
			(f8)=((f2)^(f4)^(f8)), \
			(f2)^=(f9), \
			(f4)^=(f9), \
			(f8)^=rot3(f2), \
			(f8)^=rot2(f4), \
			(f8)^rot1(f9))

uint8 AES::sbox[256];
uint8 AES::isbox[256];
uint8 AES::Rcon[30] = {
	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,
	0x1b,0x36,0x6c,0xd8,0xab,0x4d,0x9a,0x2f,
	0x5e,0xbc,0x63,0xc6,0x97,0x35,0x6a,0xd4,
	0xb3,0x7d,0xfa,0xef,0xc5,0x91,
};


unsigned char AES_xtime(uint32 x) {
	return (x & 0x80) ? (x << 1) ^ 0x1b : x << 1;
}

void AES::init() {
	uint32 t[256], i;
	uint32 x;
	for (i = 0, x = 1; i < 256; i++) {
		t[i] = x;
		x ^= (x << 1) ^ ((x >> 7) * 0x11B);
	}

	sbox[0] = 0x63;
	for (i = 0; i < 255; i++) {
		x = t[255 - i];
		x |= x << 8;
		x ^= (x >> 4) ^ (x >> 5) ^ (x >> 6) ^ (x >> 7);
		sbox[t[i]] = (x ^ 0x63) & 0xFF;
	}

	for (i = 0; i < 256; i++)
		isbox[sbox[i]] = i;
}

void AES::setKey(const uint8 *key, bool encrypt) {
	this->encrypt = encrypt;

	int i, ii;
	uint32 *W, tmp, tmp2;
	const uint8 *ip;
	int words = 8;
	
	W = ks;

	W = ks;
	i = 0;
	while (i < words)
		W[i++] = swapUInt32(*(uint32*)&key[i * 4]);
	
	ip = Rcon;
	ii = 4 * (AES_ROUNDS + 1);
	for (i = words; i<ii; i++) {
		tmp = W[i - 1];

		if ((i % words) == 0) {
			tmp2  = (uint32)sbox[(tmp	)&0xff]<< 8;
			tmp2 |= (uint32)sbox[(tmp>> 8)&0xff]<<16;
			tmp2 |= (uint32)sbox[(tmp>>16)&0xff]<<24;
			tmp2 |= (uint32)sbox[(tmp>>24)	 ];
			tmp = tmp2^(((uint32)*ip)<<24);
			ip++;
		}

		if ((i % words) == 4) {
			tmp2  = (uint32)sbox[(tmp	)&0xff];
			tmp2 |= (uint32)sbox[(tmp>> 8)&0xff]<< 8;
			tmp2 |= (uint32)sbox[(tmp>>16)&0xff]<<16;
			tmp2 |= (uint32)sbox[(tmp>>24)	 ]<<24;
			tmp = tmp2;
		}

		W[i] = W[i-words] ^ tmp;
	}
	
	if (!encrypt) {	
		uint32 *k,w,t1,t2,t3,t4;

		k = &ks[4];

		for (int i = AES_ROUNDS * 4; i > 4; i--) {
			w = *k; 			
			*k++ = inv_mix_col(w,t1,t2,t3,t4);
		}		
	}
}

void AES::prepare(void *server_nonce, void *new_nonce, bool encrypt) {
	char ns[48], sn[48], nn[64];
	memcpy(ns, new_nonce, 32);		memcpy(&ns[32], server_nonce, 16);
	memcpy(sn, server_nonce, 16);	memcpy(&sn[16], new_nonce, 32);
	memcpy(nn, new_nonce, 32);		memcpy(&nn[32], new_nonce, 32);
	
// tmp_aes_key = SHA1(new_nonce + server_nonce) + substr(SHA1(server_nonce + new_nonce), 0, 12);
	SHA1(sn, sizeof(sn), key);
	memcpy(&key[20], key, 12);
	SHA1(ns, sizeof(ns), key);
// tmp_aes_iv  = substr(SHA1(server_nonce + new_nonce), 12, 8) + SHA1(new_nonce + new_nonce) + substr(new_nonce, 0, 4);	
	memcpy(&iv[28], new_nonce, 4);
	SHA1(sn, sizeof(sn), iv);
	memcpy(iv, &iv[12], 8);
	SHA1(nn, sizeof(nn), &iv[8]);

	setKey((uint8*)&key, encrypt);
}

void AES::IGE(uint32 *data, int size) {
	uint32	*xp = (uint32*)&iv[encrypt ? 16 : 0],
			*yp = (uint32*)&iv[encrypt ? 0 : 16],
			x[4], y[4];

	void *end = (void*)((int)data + size);

	while (data < end) {
		for (int i = 0; i < 4; i++) 
			y[i] = swapUInt32((x[i] = data[i]) ^ yp[i]);
		
		if (encrypt) 			
			encryptBlock(y);
		else
			decryptBlock(y);

		for (int i = 0; i < 4; i++) {
			yp[i] = *data++ = swapUInt32(y[i]) ^ xp[i];
			xp[i] = x[i];
		}
	}
}

void AES::encryptBlock(uint32 *data) {
	uint32 tmp[4];
	uint32 tmp1, old_a0, a0, a1, a2, a3, row;
	int curr_rnd;	
	const uint32 *k = ks;

	for (row = 0; row < 4; row++)
		data[row] ^= *(k++);

	for (curr_rnd = 0; curr_rnd < AES_ROUNDS; curr_rnd++) {
		for (row = 0; row < 4; row++) {
			a0 = (uint32)sbox[(data[row%4]>>24)&0xFF];
			a1 = (uint32)sbox[(data[(row+1)%4]>>16)&0xFF];
			a2 = (uint32)sbox[(data[(row+2)%4]>>8)&0xFF]; 
			a3 = (uint32)sbox[(data[(row+3)%4])&0xFF];

			if (curr_rnd < AES_ROUNDS - 1) {
				tmp1 = a0 ^ a1 ^ a2 ^ a3;
				old_a0 = a0;
				a0 ^= tmp1 ^ AES_xtime(a0 ^ a1);
				a1 ^= tmp1 ^ AES_xtime(a1 ^ a2);
				a2 ^= tmp1 ^ AES_xtime(a2 ^ a3);
				a3 ^= tmp1 ^ AES_xtime(a3 ^ old_a0);
			}

			tmp[row] = ((a0 << 24) | (a1 << 16) | (a2 << 8) | a3);
		}

		for (row = 0; row < 4; row++)
			data[row] = tmp[row] ^ *(k++);
	}

}

void AES::decryptBlock(uint32 *data) { 	
	uint32 tmp[4];
	uint32 xt0,xt1,xt2,xt3,xt4,xt5,xt6;
	uint32 a0, a1, a2, a3, row;
	int curr_rnd;   
	const uint32 *k = ks + (AES_ROUNDS + 1) * 4;

	for (row=4; row > 0;row--)
		data[row-1] ^= *(--k);

	for (curr_rnd = 0; curr_rnd < AES_ROUNDS; curr_rnd++) {
		for (row = 4; row > 0; row--) {
			a0 = isbox[(data[(row+3)%4]>>24)&0xFF];
			a1 = isbox[(data[(row+2)%4]>>16)&0xFF];
			a2 = isbox[(data[(row+1)%4]>>8)&0xFF];
			a3 = isbox[(data[row%4])&0xFF];

			if (curr_rnd < AES_ROUNDS - 1) {
				xt0 = AES_xtime(a0^a1);
				xt1 = AES_xtime(a1^a2);
				xt2 = AES_xtime(a2^a3);
				xt3 = AES_xtime(a3^a0);
				xt4 = AES_xtime(xt0^xt1);
				xt5 = AES_xtime(xt1^xt2);
				xt6 = AES_xtime(xt4^xt5);

				xt0 ^= a1^a2^a3^xt4^xt6;
				xt1 ^= a0^a2^a3^xt5^xt6;
				xt2 ^= a0^a1^a3^xt4^xt6;
				xt3 ^= a0^a1^a2^xt5^xt6;
				tmp[row-1] = ((xt0<<24)|(xt1<<16)|(xt2<<8)|xt3);
			} else
				tmp[row-1] = ((a0<<24)|(a1<<16)|(a2<<8)|a3);
		}

		for (row = 4; row > 0; row--)
			data[row-1] = tmp[row-1] ^ *(--k);
	}	
	
}
