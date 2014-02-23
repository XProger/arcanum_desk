#include "main.h"
#include "tl.h"

#ifdef _DEBUG
	void LOG_dump(const char *data, int count) {
		for (int i = 0; i < count; i++) {
			if (i % 16 == 0)
				printf("\n %04X | ", i & 0xff);
			printf("%02X ", data[i] & 0xff);
		}
		printf("\n");
	}
#endif 

Window *Main::window;
MTP *Main::mtp;


void AES_test1() {
	FILE *f = fopen("20131011_221046.mp4", "rb");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	char *data = new char[size];
	fseek(f, 0, SEEK_SET);
	fread(data, 1, size, f);
	fclose(f);

	char key[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1};

	int t = GetTickCount();
	AES aes;
	aes.setKey((uint8*)&key[0], false);
	aes.IGE((uint32*)&data[0], size);
	printf("time: %d\n", GetTickCount() - t);
	delete data;
}

void AES_test2() {
	mpz_t t, d;
	
	AES aes;
	
	LOG("key:");
	mpz_init_set_str(t, "2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe", 16);
	int encrypted_answer_size;
	char *encrypted_answer = mpz_bytes(t, &encrypted_answer_size);
	aes.setKey((uint8*)encrypted_answer, true);	
	LOG_dump(encrypted_answer, encrypted_answer_size);

	LOG("data:");        	
	//mpz_init_set_str(d, "1a6e6c2c662e7da6501ffb62bc9e93f3", 16);	
	mpz_init_set_str(d, "3243f6a8885a308d313198a2e0370734", 16);	
	encrypted_answer = mpz_bytes(d, &encrypted_answer_size);
	LOG_dump(encrypted_answer, encrypted_answer_size);

	aes.IGE((uint32*)encrypted_answer, encrypted_answer_size);

	LOG("result:");
	LOG_dump(encrypted_answer, encrypted_answer_size);

	delete encrypted_answer;
	mpz_clear(t);
	mpz_clear(d);
}



void RSA_test() {
	const char *encrypted_data = "47e39b83fe888d1b494d8f3095d3118e190b67366f3722b0a87455d7c124ee3b03ce118df19abf1016d5f123057558e5448bcab3a2548f5e6e352ccdb3c38723054b25808c950712e9b9930d44bc2bedde17fb3b87a9d4a422bc5825559e8e289af4681bea666128d72582be84e197d45cd1b17a3e8d36f45227593ec3976737d9efec298e65bf2d252787c57ee4823eed2cf6e34d3b2f0469bacbcca02380f7c7fc5532f8700e3edf685a9366b257e870bf4aac549b4563a1b3ec4a3879648f9e9db70ff61f4aa763de4841e1b9f0fadd70a1b20553b55bbd93fe2f5e317637928bbbab3c6e1f5038d040ee8fb567a3014cfea2a307345b23b32eb6883e2c01";
	
}


void Main::init() {		
	AES::init();
	TL::init();
	RSA_test();
/*
	window = new Window();
// ...
	mtp = new MTP("173.240.5.1", 443);
	window->msgLoop();
*/
}

void Main::deinit() {
	delete mtp;
	delete window;
	TL::deinit();
}


