#include <stdio.h>
#include <string.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

int my_memcmp(unsigned char * a, unsigned char * b, unsigned int size){
	for(int i = 0; i < size; i++, a++, b++){
		printf(BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(*a), BYTE_TO_BINARY(*b));
		/*
		if(*a != *b){
			unsigned char _a = a[0];
			unsigned char _b = b[0];
			return _a - _b;
		}
		*/
	}
	return 0;
}

int main(){
	int a = 2000;
	int b = 3000;
	printf("%d\n", my_memcmp(&a, &b, sizeof(int)));
	a = 200000;
	b = 300000;
	printf("%d\n", my_memcmp(&a, &b, sizeof(int)));
	return 0;
}
