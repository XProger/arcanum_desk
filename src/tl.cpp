#include "main.h"
#include "TL.h"
#include "scheme.inc"

HashMap *TL::hashMap;
char	*TL::buf;
int		TL::pos;

int TL_TYPE_SIZE[] = { 4, 4, 8, 8, 0, 0, 16, 32, 0, 0 };

void TL::init() {	
	char *data = (char*)&DATA_CTOR[0];
	char *type = (char*)&DATA_TYPE[0];

	int count = *(int*)data;
	data += 4;
	hashMap = new HashMap(count);	
	while (count--) {
		int offset = *(short*)&data[4];
		hashMap->put(*(int*)data, &type[offset]);
		data += 6;
	}

	buf = new char[TL_BUF_SIZE];
}

void TL::deinit() {
	delete buf;
	delete hashMap;
}
/*
void TL::deserialize(const char *data) {	
	int id = *(int*)data;
	char *info = (char*)hashMap->get(id);
	if (!info) {
		LOG("error: not found object 0x%08x", id);
		return;
	}

	char *typeName[] = {"bool", "int", "long", "double", "string", "bytes", "int128", "int256", "vector", "object"};

	int count = info[0];	
	info++;
	LOG("params: %d\n", count); 
	for (int i = 0; i < count; i++) {
		int type = *(info++);
		if (type == TL_VAR_VECTOR) 
			LOG(" type: %s<%s>\n", typeName[type], typeName[*(info++)]);
		else
			LOG(" type: %s\n", typeName[type]);
	}

}
*/
void TL::begin(int offset) {
	pos = offset;
}

void TL::end() {
	//
}

void TL::put(const void *value, int size) {
	switch (size) {
		case 0	: return;
		case 1	: buf[pos] = *(char*)value; break;
		case 4	: *(int*)&buf[pos] = *(int*)value; break;
		case 8	: *(__int64*)&buf[pos] = *(__int64*)value; break;
		default	: memcpy(&buf[pos], value, size);
	}
	pos += size;
}

void TL::_bool(bool value) {
	int i = value ? TL_BOOLTRUE : TL_BOOLFALSE;
	_int(i);
}

void TL::_int(int value) {
	put(&value, sizeof(value));
}

void TL::_long(__int64 value) {
	put(&value, sizeof(value));
}

void TL::_double(double value) {
	put(&value, sizeof(value));
}

void TL::_string(const char *value) {
	int size = value ? strlen(value) : 0;
	_bytes(value, size);
}

void TL::_bytes(const char *value, int size) {	
	int p = pos;
	if (size < 0xfe)
		put(&size, 1);
	else
		_int((size << 8) | 0xfe);		
	put(value, size);

	while (pos % 4)
		buf[pos++] = 0;
	//pos += (p - pos) & 3;	// 4-bytes align
}

void TL::_bytes(mpz_t value) {
	int size;
	char *p = mpz_bytes(value, &size);
	_bytes(p, size);
	delete p;
}

void TL::_int128(const Int128 &value) {
	put(&value, sizeof(value));
}

void TL::_int256(const Int256 &value) {
	put(&value, sizeof(value));
}

void TL::_vector(int count) {
	_int(TL_VECTOR);
	_int(count);
}

void TL::_object(int id) {
	_int(id);
}


TL::Object::Object(const char *data) : count(0), params(0) {
	id = *(int*)data;
	size = 4;
	char *info = (char*)hashMap->get(id);
	if (!info) {
		LOG("error: not found object 0x%08x", id);
		return;
	}

	count = *(info++);
	parseParams(data, info, 0);
}

TL::Object::~Object() {
	for (int i = 0; i < count; i++)
		switch (params[i].type) {
			case TL_VAR_VECTOR : 
				delete (Vector*)params[i].value;
				break;
			case TL_VAR_OBJECT :
				delete (Object*)params[i].value;
				break;
		}
	delete params;
}

void TL::Object::parseParams(const char *data, char *info, int type) {
	params = new Param[count];
	Param *p = params;

	for (int i = 0; i < count; i++) {
		p->type = info ? *(info++) : type;
		switch (p->type) {
			case TL_VAR_BOOL	:
			case TL_VAR_INT		:
			case TL_VAR_LONG	:
			case TL_VAR_DOUBLE	:
			case TL_VAR_INT128	:
			case TL_VAR_INT256	:
				p->value = (void*)&data[size];
				size += TL_TYPE_SIZE[p->type];
				break;
			case TL_VAR_STRING	:
			case TL_VAR_BYTES	:
				p->value = (void*)&data[size];
				size += ((String*)p->value)->size();
				break;
			case TL_VAR_VECTOR	:	// !!! vector of vector not supported
			case TL_VAR_OBJECT	:
				p->value = p->type == TL_VAR_VECTOR ? new Vector(&data[size], *(info++)) : new Object(&data[size]);
				size += ((Object*)p->value)->size;
				break;
		}
		p++;
	}
}

TL::Vector::Vector(const char *data, int type) : Object(*(int*)data) {
	count = *(int*)&data[size];
	size += 4;
	parseParams(data, 0, type);
}