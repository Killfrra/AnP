#ifndef CUSTOM_LIST
#define CUSTOM_LIST

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

    free(el);
}

#endif