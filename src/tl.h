#ifndef TL_H
#define TL_H

#include <string.h>
#include "utils.h"

#define TL_VAR_BOOL		0
#define TL_VAR_INT		1
#define TL_VAR_LONG		2
#define TL_VAR_DOUBLE	3
#define TL_VAR_STRING	4
#define TL_VAR_BYTES	5
#define TL_VAR_INT128	6
#define TL_VAR_INT256	7
#define TL_VAR_VECTOR	8
#define TL_VAR_OBJECT	9

#define TL_BOOLFALSE	0xbc799737
#define TL_BOOLTRUE		0x997275b5
#define TL_VECTOR		0x1cb5c415

// TL authorization
#define TL_req_pq						0x60469778
#define TL_resPQ						0x05162463
#define	TL_p_q_inner_data				0x83c95aec
#define TL_req_DH_params				0xd712e4be
#define	TL_server_DH_params_fail		0x79cb045d
#define	TL_server_DH_params_ok			0xd0e8075c
#define	TL_server_DH_inner_data			0xb5890dba
#define	TL_client_DH_inner_data			0x6643b654
#define TL_set_client_DH_params			0xf5045f1f
#define TL_dh_gen_ok					0x3bcbf734
#define TL_dh_gen_retry					0x46dc1fb9
#define TL_dh_gen_fail					0xa69dae02

struct TL {

	struct Int128 {
		char data[16];

		Int128() {}
		Int128(void *value) {
			memcpy(&data, value, sizeof(data));
		}

		Int128* copy() {
			return new Int128(&data);
		}
		
		void random() {
			for (int i = 0; i < sizeof(data); i++)
				data[i] = rand() % 0xff;
		}
	};

	struct Int256 {
		char data[32];

		Int256() {}
		Int256(void *value) {
			memcpy(&data, value, sizeof(data));
		}

		Int256* copy() {
			return new Int256(&data);
		}

		void random() {
			for (int i = 0; i < sizeof(data); i++)
				data[i] = rand() % 0xff;
		}
	};

	struct String {
		unsigned char data;
		
		int size() {
			return ((data != 0xfe ? 1 : 4) + length() + 3) & ~3;
		}

		int length() {			
			return data != 0xfe ? data : *((int*)&data) >> 8;
		}

		char *ptr() {
			return (char*)&data + (data != 0xfe ? 1 : 4);
		}

		char* copy() {
			int len = length();
			char *str = new char[len + 1];
			memcpy(str, ptr(), len);
			str[len] = 0;
			return str;
		}
	};

	struct Param {
		int type;
		void *value;
	};

	struct Vector;

	struct Object {
		int id;
		int size;
		int count;
		Param *params;

		Object(int id) : id(id), size(4), count(0), params(0) {} 
		Object(const char *data);
		~Object();
		void parseParams(const char *data, char *info, int type);
		
		void*		get(int idx)		const	{ return params[idx].value; }
		bool		_bool(int idx)		const	{ return _int(idx) == TL_BOOLTRUE; }
		int& 		_int(int idx)		const	{ return *(int*)get(idx); }
		__int64&	_long(int idx)		const	{ return *(_int64*)get(idx); }
		double&		_double(int idx)	const	{ return *(double*)get(idx); }
		String*		_string(int idx)	{ return (String*)get(idx); }
		String*		_bytes(int idx)		{ return _string(idx); }
		Int128*		_int128(int idx)	{ return (Int128*)get(idx); }
		Int256*		_int256(int idx)	{ return (Int256*)get(idx); }
		Vector*		_vector(int idx)	{ return (Vector*)get(idx); }
		Object*		_object(int idx)	{ return (Object*)get(idx); }
	};

	struct Vector : Object {
		Vector(const char *data, int type);
	};

	static HashMap *hashMap;
	static int pos;
	static char *buf;

	static void init();
	static void deinit();

	static void begin(int offset = 0);	
	static void end();

	static void		put(const void *value, int size);
	static void		_bool(bool value);
	static void		_int(int value);
	static void		_long(__int64 value);
	static void		_double(double value);
	static void		_string(const char *value);
	static void		_bytes(const char *value, int size);
	static void		_bytes(mpz_t value);
	static void		_int128(const Int128 &value);
	static void		_int256(const Int256 &value);
	static void		_vector(int count);
	static void		_object(int id);
/*
	static bool		getBool();
	static int		getInt();
	static __int64	getLong();
	static double	getDouble();
	static String*	getString();
	static char*	getBytes(int *size);
	static Int128*	getInt128();
	static Int256*	getInt256();
*/
};


#endif