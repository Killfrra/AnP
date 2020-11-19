#include<stdio.h>
#include<string.h>

int main(){
    int a = 40000;
    int b = 30000;
    printf("%d\n", memcmp(&a, &b, sizeof(int)));
    return 0;
}