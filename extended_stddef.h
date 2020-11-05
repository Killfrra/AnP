#ifndef CUSTOM_DEFINES 
#define CUSTOM_DEFINES

#include <stdio.h>
#include <stddef.h>

#define loop while(1)
#define new(T) malloc(sizeof(T))
#define len(x)  (sizeof(x) / sizeof((x)[0]))
#define TRUE 1
#define FALSE 0

typedef struct {
    char d, m;
    short y;
} Date;

void print_date(Date date){
    printf(
        "%02u.%02u.%04u",
        (short) date.d,
        (short) date.m,
        (short) date.y
    );
}

#endif