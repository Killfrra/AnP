#ifndef CUSTOM_DEFINES 
#define CUSTOM_DEFINES

#include <stdio.h>
#include <stddef.h>

#define loop while(1)
#define new(T) malloc(sizeof(T))
#define len(x)  (sizeof(x) / sizeof((x)[0]))

typedef struct {
    unsigned char d, m;
    unsigned short y;
} Date;

void print_date(Date date){
    printf(
        "%02hu.%02hu.%04hu",
        (unsigned short) date.d,
        (unsigned short) date.m,
        (unsigned short) date.y
    );
}

#endif