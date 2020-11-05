#include <stdio.h>
#define len(a) (sizeof(a) / sizeof(a[0]))

typedef struct list_element {
    int val;
    struct list_element * prev, * next;
} ListElement;

ListElement l[] = {
    { 1,  NULL, &l[1] },
    { 8, &l[0], &l[2] },
    { 3, &l[1], &l[3] },
    { 4, &l[2], &l[4] },
    { 0, &l[3], &l[5] },
    { 5, &l[4], &l[6] },
    { 7, &l[5], NULL  }
};

int l_len = 7;

int a[] = { 1, 8, 3, 4, 0, 5, 7 };

void print(){
    for(int i = 0; i < len(a); i++)
        printf("%d ", (int) a[i]);
    putchar('\n');
}

void recursion(int from, int to){
    //printf("r %i %i\n", from, to);
    int _len = to - from + 1;
    if(_len > 2){
        recursion(from, _len / 2 - 1);
        recursion(_len / 2, to);
    } else
        printf("%i %i %i\n", a[from], a[from + 1], a[from + 2]);
}

int main(){

    print();
    recursion(0, len(a) - 1);

    /*
    float median = 0;
    for(int i = 0; i < len(a); i++)
        median += a[i];
    median /= len(a);

    printf("median: %f\n", median);

    int i = 0, j = len(a) - 1;
    for(; i < j; i++)
        if(a[i] > median){
            while(a[j] >= median)
                j--;
            int tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            j--;
        }

    print();
    printf("%i %i\n", (int) i, (int) j);
    */

    return 0;
}