#include <stdio.h>
#define len(a) (sizeof(a) / sizeof(a[0]))
#define connect(a, b)  { a->next = b; b->prev = a; }
#define connect3(a, b, c) { connect(a, b); connect(b, c); }

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
    { 7, &l[5], NULL  },
    //{ 2, &l[5], NULL  }
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
    ListElement * cur, * cur1, * cur2;
    if(l1.first->val < l2.first->val){
        cut.first = cur = l1.first;
        cur1 = l1.first->next;
        cur2 = l2.first;
    } else {
        cut.first = cur = l2.first;
        cur1 = l1.first;
        cur2 = l2.first->next;
    }

    while(1){
        if(cur1->val < cur2->val){
            connect(cur, cur1);
            cur = cur1;
            if(cur == l1.last){
                connect(cur, cur2);
                cut.last = l2.last;
                return cut;
            } else
                cur1 = cur1->next;
        } else {
            connect(cur, cur2);
            cur = cur2;
            if(cur == l2.last){
                connect(cur, cur1);
                cut.last = l1.last;
                return cut;
            } else
                cur2 = cur2->next;
        }
    }
}

Cut recursion(ListElement * first, int len){
    //printf("r %i %i\n", from, to);
    if(len > 3){
        int half_len = len / 2;
        Cut cut1 = recursion(first, half_len);
        Cut cut2 = recursion(cut1.next, len - half_len); // uses next
        return merge(cut1, cut2);
    } else if(len > 2){
        Cut cut = recursion(first->next, 2);
        if(first->val < cut.first->val){
            connect(first, cut.first);
            cut.first = first;
        } else if(first->val < cut.last->val){
            connect3(cut.first, first, cut.last);
        } else {
            connect(cut.last, first);
            cut.last = first;
        }
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

void merge_sort(ListElement * first, int len){
    Cut cut = recursion(first, len);
    head = cut.first;
    tail = cut.last;
    head->prev = NULL;
    tail->next = NULL;
}

int main(){

    print();
    merge_sort(head, list_len);
    print();

    return 0;
}