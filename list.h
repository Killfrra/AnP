#ifndef CUSTOM_LIST
#define CUSTOM_LIST

#include <string.h>
#include <stdlib.h>
#include "extended_stddef.h"
#include "extended_conio.h"

#define FILEDATA_NAME_LEN 32

typedef struct {
    char group_name[6 + 1]; // 7
    int gradebook_number; // 4
    char full_name[FILEDATA_NAME_LEN]; // 32
    char gender, education_form; // 1 + 1
    Date birth_date, admission_date; // 4 + 4
    short USE_score; // 2
} FileData;

typedef struct list_element {
    FileData data;
    struct list_element * link[4];
} ListElement;

enum { SHOW = 0, SEARCH = 1 } link_layer = SHOW;

#define DIR(d) link[d * 2 + link_layer]
#define PREV   link[0 + link_layer]
#define NEXT   link[2 + link_layer]
#define VAL    data.USE_score       //TODO: compare_by_val(a, b)

#define connect(a, b)  { a->NEXT = b; b->PREV = a; }
#define connect3(a, b, c) { connect(a, b); connect(b, c); }

/*
char read_string(char enter_dir, short posx, char * dest, Field field);
char read_fixed_int(char enter_dir, short posx, unsigned int * dest, Field field);
char read_char(char enter_dir, short posx, char * dest, Field field);
char read_fixed_date(char enter_dir, short posx, Date * dest, Field field);
char read_fixed_short(char enter_dir, short posx, unsigned short * dest, Field field);
*/

Field list_element_fields[] = {
	{ read_string      , "Шифр группы", offsetof(FileData, group_name), 6, { allow_digits: TRUE }, .size = 6},
	{ read_fixed_int   , "Номер зачетной книжки", offsetof(FileData, gradebook_number), 6, .size = sizeof(int) },
	{ read_char        , "Пол", offsetof(FileData, gender), 1, { values: "\2mf" }, .size = 1 },
	{ read_char        , "Форма обучения", offsetof(FileData, education_form), 1, { values: "\3ozd" }, .size = 1 },
	{ read_fixed_date  , "Дата рождения", offsetof(FileData, birth_date), 10, .size = sizeof(Date) },
	{ read_fixed_date  , "Дата поступления", offsetof(FileData, admission_date), 10, .size = sizeof(Date) },
	{ read_fixed_short , "Балл ЕГЭ", offsetof(FileData, USE_score), 3, .size = sizeof(short) },
    { read_string      , "ФИО", offsetof(FileData, full_name), FILEDATA_NAME_LEN, { allow_digits: FALSE }, .size = FILEDATA_NAME_LEN }
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

ListElement * heads[2] = { 0 };
ListElement * tails[2] = { 0 };
#define HEAD heads[link_layer]
#define TAIL tails[link_layer]

unsigned int list_lengths[2] = { 0 };
#define list_len list_lengths[link_layer]

void list_add(ListElement * el){
    
    if(!HEAD){
        HEAD = el;
        HEAD->PREV = NULL;  
    } else {
        TAIL->NEXT = el;
        el->PREV = TAIL;
    }
    TAIL = el;
    TAIL->NEXT = NULL;

    list_len++;
}

void list_remove(ListElement * el){
    
    if(el == HEAD){
        HEAD = el->NEXT;
        if(HEAD)
            HEAD->PREV = NULL;
    } else
        el->PREV->NEXT = el->NEXT;

    if(el == TAIL){
        TAIL = el->PREV;
        if(TAIL)
            TAIL->NEXT = NULL;
    } else
        el->NEXT->PREV = el->PREV;

    list_len--;
    
    if(link_layer == SHOW)
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
    Cut cut = recursion(HEAD, list_len);
    HEAD = cut.first;
    TAIL = cut.last;
    HEAD->PREV = NULL;
    TAIL->NEXT = NULL;
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
    HEAD = list;
    TAIL = &list[len(list) - 1];
    list_len = len(list);
    }

    int scores[] = { 200, 300, 500, 350, 349, 250 };
    for(unsigned char i = 0; i < len(scores); i++){
        ListElement * last_readed = new(ListElement);
        last_readed->data.USE_score = scores[i];
        list_add(last_readed);
    }

    for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
        element_print(cur);
    merge_sort();
    for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
        element_print(cur);
    return 0;
}
*/
#endif