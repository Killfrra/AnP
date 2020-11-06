#ifndef CUSTOM_LIST
#define CUSTOM_LIST

#include <string.h>
#include <stdlib.h>
#include "extended_stddef.h"

typedef struct {
    char group_name[6 + 1]; // 7
    int gradebook_number; // 4
    char full_name[32];     // 32
    char gender, education_form; // 1 + 1
    Date birth_date, admission_date; // 4 + 4
    short USE_score; // 2
} FileData;

typedef struct list_element {
    FileData data;
    struct list_element * link[4];
} ListElement;

enum { SHOW = 0, SEARCH = 2 } link_layer = SHOW;

#define DIR(d) link[d + link_layer]
#define PREV   link[0 + link_layer]
#define NEXT   link[1 + link_layer]
#define VAL    data.USE_score       //TODO: compare_by_val(a, b)

#define connect(a, b)  { a->NEXT = b; b->PREV = a; }
#define connect3(a, b, c) { connect(a, b); connect(b, c); }

typedef struct {
    char shortcut;
    char * name;
    char type;
    size_t offset;
    char size;
    union {
        char values[5];
        char allow_digits;
    } prop;
} Field;

Field list_element_fields[] = {
	{ 'c', "Шифр группы", 's', offsetof(FileData, group_name), 6, { allow_digits: TRUE }},
	{ 'i', "Номер зачетной книжки", 'i', offsetof(FileData, gradebook_number), 6 },
	{ 'g', "Пол", 'c', offsetof(FileData, gender), 1, { values: "\2mf" } },
	{ 'f', "Форма обучения", 'c', offsetof(FileData, education_form), 1, { values: "\3ozd" } },
	{ 'b', "Дата рождения", 'd', offsetof(FileData, birth_date), 10 },
	{ 'e', "Дата поступления", 'd', offsetof(FileData, admission_date), 10 },
	{ 's', "Балл ЕГЭ", 'h', offsetof(FileData, USE_score), 3},
    { 'n', "ФИО", 's', offsetof(FileData, full_name), sizeof(((FileData*) 0)->full_name), { allow_digits: FALSE } }
};

void element_print(ListElement * cur){
	FileData * _ = &cur->data;
	printf("%-6s %06u %c %c ", &_->group_name[1], _->gradebook_number, _->gender, _->education_form);
    print_date(_->birth_date);
    putchar(' ');
    print_date(_->admission_date);
    printf(" %03hu %s\n", _->USE_score, &_->full_name[1]);
}

void element_zerofill(ListElement * elem){
    memset(elem, 0, sizeof(ListElement));
    elem->data.gender = elem->data.education_form = ' ';
}

ListElement * head = NULL; //TODO: head/tail for SEARCH layer
ListElement * tail = NULL;
unsigned int list_len = 0;

void list_add(ListElement * el){
    
    if(!head){
        head = el;
        head->PREV = NULL;  
    } else {
        tail->NEXT = el;
        el->PREV = tail;
    }
    tail = el;
    tail->NEXT = NULL;

    list_len++;
}

void list_remove(ListElement * el){
    
    if(el == head){
        head = el->NEXT;
        if(head)
            head->PREV = NULL;
    } else
        el->PREV->NEXT = el->NEXT;

    if(el == tail){
        tail = el->PREV;
        if(tail)
            tail->NEXT = NULL;
    } else
        el->NEXT->PREV = el->PREV;

    list_len--;
    free(el);
}

typedef struct {
    ListElement * first, * last;
    ListElement * next; // указатель на начало следующего отрезка
} Cut;

Cut merge(Cut l1, Cut l2){
    Cut l[] = { l1, l2 };
    Cut cut = { .next = l2.next };
    ListElement * cur, * cur_[2];
    
    unsigned char min = l1.first->VAL >= l2.first->VAL;
    cut.first = cur = l[min].first;
    cur_[min] = cur->NEXT;
    cur_[!min] = l[!min].first;
    
    while(1){
        unsigned char min = cur_[0]->VAL >= cur_[1]->VAL;
        connect(cur, cur_[min]);
        cur = cur_[min];
        if(cur == l[min].last){
            connect(cur, cur_[!min]);
            cut.last = l[!min].last;
            return cut;
        } else
            cur_[min] = cur->NEXT;
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
        Cut cut = recursion(first->NEXT, 2);
        if(first->VAL < cut.first->VAL){
            connect(first, cut.first);
            cut.first = first;
        } else if(first->VAL < cut.last->VAL){
            connect3(cut.first, first, cut.last);
        } else {
            connect(cut.last, first);
            cut.last = first;
        }
        return cut;
    } else {
        Cut cut = { first, first->NEXT };
        cut.next = cut.last->NEXT;
        if(cut.first->VAL > cut.last->VAL){
            ListElement * tmp = cut.first;
            cut.first = cut.last;
            cut.last = tmp;

            cut.first->NEXT = cut.last;
            cut.last->PREV = cut.first;
        }
        return cut;
    }
}

//WARN: list_len should be >= 2
void merge_sort(){
    Cut cut = recursion(head, list_len);
    head = cut.first;
    tail = cut.last;
    head->PREV = NULL;
    tail->NEXT = NULL;
}
/*
int main(){
    {
    ListElement list[] = {
        { { .USE_score = 200 }, {     NULL, &list[1] } },
        { { .USE_score = 300 }, { &list[0], &list[2] } },
        { { .USE_score = 500 }, { &list[1], &list[3] } },
        { { .USE_score = 350 }, { &list[2], &list[4] } },
        { { .USE_score = 349 }, { &list[3], &list[5] } },
        { { .USE_score = 250 }, { &list[4], NULL     } }
    };
    head = list;
    tail = &list[len(list) - 1];
    list_len = len(list);
    }

    int scores[] = { 200, 300, 500, 350, 349, 250 };
    for(unsigned char i = 0; i < len(scores); i++){
        ListElement * last_readed = new(ListElement);
        last_readed->data.USE_score = scores[i];
        list_add(last_readed);
    }

    for(ListElement * cur = head; cur; cur = cur->NEXT)
        element_print(cur);
    merge_sort();
    for(ListElement * cur = head; cur; cur = cur->NEXT)
        element_print(cur);
    return 0;
}
*/
#endif