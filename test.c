#include <stdio.h>
#define len(a) (sizeof(a) / sizeof(a[0]))

typedef struct list_element {
    int val;
    struct list_element * prev, * next;
} ListElement;

ListElement l[] = {
    { 1,  NULL, &l[1] },
    { 8, &l[0], &l[2] },
    { 4, &l[1], &l[3] },
    { 3, &l[2], &l[4] },
    { 0, &l[3], &l[5] },
    { 5, &l[4], &l[6] },
    { 7, &l[5], &l[7] },
    { 2, &l[6], NULL  }
};

int list_len = len(l);
ListElement * head = &l[0];
ListElement * tail = &l[len(l) - 1];

void print(){
    int i = 0;
    for(ListElement * cur = head; cur; cur = cur->next, i++){
        if(i == list_len){
            puts("...");
            return;
        }
        printf("%d ", cur->val);
    }
    putchar('\n');
}

typedef struct {
    ListElement * first, * last;
    ListElement * next; // указатель на начало следующего отрезка
} Cut;

Cut merge(Cut l1, Cut l2){
    Cut cut = { .next = l2.next };
    ListElement * cur;
    if(l1.first->val < l2.first->val){
        cut.first = l1.first;
        cur = l2.first;        
    } else {
        cut.first = l2.first;
        cur = l1.first;
    }

    ListElement * cur1 = l1.first->next;
    ListElement * cur2 = l2.first->next;

    cut.first->next = cur;
    cur->prev = cut.first;

    while(1){
        if(cur1->val < cur2->val){
            cur->next = cur1;
            cur1->prev = cur;
            cur = cur1;
            if(cur == l1.last){
                cur->next = cur2;
                cur2->prev = cur;
                cut.last = l2.last;
                return cut;
            } else
                cur1 = cur1->next;
        } else {
            cur->next = cur2;
            cur2->prev = cur;
            cur = cur2;
            if(cur == l2.last){
                cur->next = cur1;
                cur1->prev = cur;
                cut.last = l1.last;
                return cut;
            } else
                cur2 = cur2->next;
        }
    }
}

Cut recursion(ListElement * first, int len){
    //printf("r %i %i\n", from, to);
    if(len > 2){
        int half_len = len / 2;
        Cut cut1 = recursion(first, half_len);
        Cut cut2 = recursion(cut1.next, len - half_len);
        return merge(cut1, cut2);
    } else if(len == 1){
        Cut cut = { first, first, first->next };
        return cut;
    } else {
        Cut cut = { first, first->next };
        cut.next = cut.last->next;
        if(cut.first->val > cut.last->val){
            ListElement * tmp = cut.first;
            cut.first = cut.last;
            cut.last = tmp;

            cut.first->next = cut.last;
            cut.last->prev = cut.first;
        }
        return cut;
    }
}

int main(){

    print();
    recursion(head, list_len);
    print();

    return 0;
}