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
    { 7, &l[5], &l[7] },
    { 2, &l[5], &l[8] },
    { 0, &l[6], &l[9] }
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
    #define l (&l1)
    Cut cut = { .next = l2.next };
    ListElement * cur, * cur_[2];
    
    char min = l1.first->val >= l2.first->val;
    cut.first = cur = l[min].first;
    cur_[min] = cur->next;
    cur_[!min] = l[!min].first;
    
    while(1){
        char min = cur_[0]->val >= cur_[1]->val;
        connect(cur, cur_[min]);
        cur = cur_[min];
        if(cur == l[min].last){
            connect(cur, cur_[!min]);
            cut.last = l[!min].last;
            return cut;
        } else
            cur_[min] = cur->next;
    }
    #undef l
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