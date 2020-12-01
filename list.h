#ifndef CUSTOM_LIST
#define CUSTOM_LIST

#include <string.h>
#include <stdlib.h>
#include "extended_stddef.h"
#include "extended_conio.h"

#define FILEDATA_NAME_LEN 32

typedef struct {
    char group_name[6 + 2]; // 8
    unsigned int gradebook_number; // 4
    char full_name[FILEDATA_NAME_LEN]; // 32
    char gender, education_form; // 1 + 1
    Date birth_date, admission_date; // 4 + 4
    unsigned short USE_score; // 2
} FileData;

typedef struct list_element {
    FileData data;
    struct list_element * link[4];
} ListElement;

enum { SHOW = 0, SEARCH = 1 } link_layer = SHOW;

#define DIR(d) link[d * 2 + link_layer]
#define PREV   link[0 + link_layer]
#define NEXT   link[2 + link_layer]

#define connect(a, b)  { a->NEXT = b; b->PREV = a; }
#define connect3(a, b, c) { connect(a, b); connect(b, c); }

/*
char read_string(char enter_dir, short posx, char * dest, Field field);
char read_fixed_int(char enter_dir, short posx, unsigned int * dest, Field field);
char read_char(char enter_dir, short posx, char * dest, Field field);
char read_fixed_date(char enter_dir, short posx, Date * dest, Field field);
char read_fixed_short(char enter_dir, short posx, unsigned short * dest, Field field);
*/

#define compare_func(name, type) int name(void * a, void * b){ \
    return *(type *) a - *(type *) b; \
}

compare_func(intcmp, int)
compare_func(chrcmp, char)
compare_func(shrcmp, short)

// ������  �������  ���  �����  ��������    �����������  ���  ���                              //
// ......  ......    .     .    00.00.0000  00.00.0000   ...  ................................ //

#define field(x, l, s, r, c, n, o, p) { x, l, s, r, c, n, offsetof(FileData, o), p }
Field list_element_fields[] = {
	field( 1,  6,             6, read_string     ,  strcmp, "������"     , group_name      , { allow_digits: TRUE }),
	field( 9,  6,   sizeof(int), read_fixed_int  ,  intcmp, "�������"    , gradebook_number, {0}),
	field(19,  1,             1, read_char       ,  chrcmp, "���"        , gender          , { values: "\2mf"  }),
	field(25,  1,             1, read_char       ,  chrcmp, "�����"      , education_form  , { values: "\3ozd" }),
	field(30, 10,  sizeof(Date), read_fixed_date ,  intcmp, "��������  " , birth_date      , {0}),
	field(42, 10,  sizeof(Date), read_fixed_date ,  intcmp, "�����������", admission_date  , {0}),
	field(55,  3, sizeof(short), read_fixed_short,  shrcmp, "���"        , USE_score       , {0}),
    field(60, 32,            32, read_string     ,  strcmp, "���"        , full_name       , { allow_digits: FALSE })
};
#undef field

void element_print(ListElement * cur){
	FileData * _ = &cur->data;
	printf(" %-6s  %06u    %c     %c    ", &_->group_name[1], _->gradebook_number, _->gender, _->education_form);
    print_date(_->birth_date);
    printf("  ");
    print_date(_->admission_date);
    printf("   %03hu  %s\n", _->USE_score, &_->full_name[1]);
}

void element_print_to_txt(FILE * f, ListElement * cur){
	FileData * _ = &cur->data;
	fprintf(
        f, "%-6s %06u %c %c %02hu.%02hu.%04hu %02hu.%02hu.%04hu %3hu %s\n",
        &_->group_name[1], _->gradebook_number, _->gender, _->education_form,
        (short) _->birth_date.d, (short) _->birth_date.m, _->birth_date.y,
        (short) _->admission_date.d, (short) _->admission_date.m, _->admission_date.y,
        _->USE_score, &_->full_name[1]
    );
}
/*
void element_read_from_txt(FILE * f, ListElement * cur){
	FileData * _ = &cur->data;
	fscanf(
        f, "%6s %6u %c %c %2hhu.%2hhu.%4hu %2hhu.%2hhu.%4hu %3hu",
        &_->group_name[1], &_->gradebook_number, &_->gender, &_->education_form,
        &_->birth_date.d, &_->birth_date.m, &_->birth_date.y,
        &_->admission_date.d, &_->admission_date.m, &_->admission_date.y,
        _->USE_score
    );
    // &_->full_name[1]
}
*/
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

CompareFunc list_element_compare_func;
size_t field_to_compare_by_offset;
int list_element_compare(ListElement * a, ListElement * b){
    return list_element_compare_func((char *) a + field_to_compare_by_offset, (char *) b + field_to_compare_by_offset);
}

typedef struct {
    ListElement * first, * last;
    ListElement * next; // ��������� �� ������ ���������� �������
} Cut;

Cut merge(Cut l1, Cut l2){
    Cut l[] = { l1, l2 };
    Cut cut = { .next = l2.next };
    ListElement * cur, * cur_[2];
    
    unsigned char min = list_element_compare(l1.first, l2.first) >= 0;
    cut.first = cur = l[min].first;
    cur_[min] = cur->NEXT;
    cur_[!min] = l[!min].first;
    
    while(1){
        unsigned char min = list_element_compare(cur_[0], cur_[1]) >= 0;
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
    } else if(len == 3){
        Cut cut = recursion(first->NEXT, 2);
        if(list_element_compare(first, cut.first) <= 0){
            connect(first, cut.first);
            cut.first = first;
        } else if(list_element_compare(first, cut.last) < 0){
            connect3(cut.first, first, cut.last);
        } else {
            connect(cut.last, first);
            cut.last = first;
        }
        return cut;
    } else /* if(len == 2) */ {
        Cut cut = { first, first->NEXT };
        cut.next = cut.last->NEXT;
        if(list_element_compare(cut.first, cut.last) > 0){
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
void merge_sort(unsigned char field_id){
    Field field = list_element_fields[field_id];
    field_to_compare_by_offset = field.offset;
    list_element_compare_func = field.comp_func;
    /*
    //TODO: wrap into func?
    if((char *) field.read_func == (char *) read_string){
        field_to_compare_by_offset++;
        field_to_compare_by_size--;
    }
    */
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
