#ifndef CUSTOM_LIST
#define CUSTOM_LIST

#include <string.h>
#include <stdlib.h>
#include "extended_stddef.h"
#include "extended_conio.h"

#define FILEDATA_NAME_LEN 32
#define AUTOSAVE_FILE_NAME "test.bin"

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
    struct list_element * link[4]; // 4 * 4
} ListElement;

typedef enum { SHOW = 0, SEARCH = 1 } ListLinkLayer;
ListLinkLayer link_layer = SHOW;

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

int cmpdte(Date * a, Date * b){
	if(a->y == b->y){
		if(a->m == b->m)
			return a->d - b->d;
		return a->m - b->m;
	}
	return a->y - b->y;
}

// Группа  Зачетка  Пол  Форма  Рождение    Поступление  ЕГЭ  ФИО                              //
// ......  ......    .     .    00.00.0000  00.00.0000   ...  ................................ //

#define field(x, l, s, r, c, n, o, p) { x, l, s, r, c, n, offsetof(FileData, o), p }
Field list_element_fields[] = {
	field( 1,  6,             6, read_string     ,  strcmp, "Группа"     , group_name      , { allow: ALLOW_DIGITS }),
	field( 9,  6,   sizeof(int), read_fixed_int  ,  intcmp, "Зачетка"    , gradebook_number, {0}),
	field(19,  1,             1, read_char       ,  chrcmp, "Пол"        , gender          , { values: "\2mf"  }),
	field(25,  1,             1, read_char       ,  chrcmp, "Форма"      , education_form  , { values: "\3ozd" }),
	field(30, 10,  sizeof(Date), read_fixed_date ,  cmpdte, "Рождение  " , birth_date      , {0}),
	field(42, 10,  sizeof(Date), read_fixed_date ,  cmpdte, "Поступление", admission_date  , {0}),
	field(55,  3, sizeof(short), read_fixed_short,  shrcmp, "ЕГЭ"        , USE_score       , {0}),
    field(60, 32,            32, read_string     ,  strcmp, "ФИО"        , full_name       , { allow: ALLOW_NOTHING })
};
#undef field

ListElement * first_alloc_begin = NULL;
ListElement * first_alloc_end = NULL;
ListElement * freeded_elements = NULL;
ListElement * last_readed;

ListElement * list_element_new(){
	if(freeded_elements != NULL){
		ListElement * popped = freeded_elements;
		freeded_elements = popped->link[2]; // 2 = SHOW NEXT
		return popped;
	}
	return new(ListElement);
}

void list_element_free(ListElement * el){
	ListElement * head = freeded_elements;
	freeded_elements = el;
	el->link[2] = head; // 2 = SHOW NEXT
}

void element_print(ListElement * cur){
	FileData * _ = &cur->data;
	printf(" %-6s  %06u    %c     %c    ", &_->group_name[1], _->gradebook_number, _->gender, _->education_form);
    print_date(_->birth_date);
    printf("  ");
    print_date(_->admission_date);
    printf("   %03hu  %-79s\n", _->USE_score, &_->full_name[1]); //TODO: unhardcode name len (80)
}

// returns negative number on falture
int element_print_to_txt(FILE * f, ListElement * cur){
	FileData * _ = &cur->data;
	return fprintf(
        f, "%-6s %06u %c %c %02hu.%02hu.%04hu %02hu.%02hu.%04hu %3hu %s\n",
        &_->group_name[1], _->gradebook_number, _->gender, _->education_form,
        (short) _->birth_date.d, (short) _->birth_date.m, _->birth_date.y,
        (short) _->admission_date.d, (short) _->admission_date.m, _->admission_date.y,
        _->USE_score, &_->full_name[1]
    );
}

int element_read_from_txt(FILE * f, ListElement * cur){
	FileData * _ = &cur->data;
    {
        int ret = fscanf( //TODO: fix bug with empty gender and form
            f, "%6c %u %c %c %hhu.%hhu.%hu %hhu.%hhu.%hu %hu ",
            &_->group_name[1], &_->gradebook_number, &_->gender, &_->education_form,
            &_->birth_date.d, &_->birth_date.m, &_->birth_date.y,
            &_->admission_date.d, &_->admission_date.m, &_->admission_date.y,
            &_->USE_score
        );
        
        if(ret <= 0)
            return -1;
        
        unsigned char i = 6;
        for(; i > 0 && _->group_name[i] == ' '; i--);
        _->group_name[0] = i;
    }{
        char * ret = fgets(&_->full_name[1], FILEDATA_NAME_LEN, f);
        if(ret == NULL)
            return -1;

        unsigned char i = 1;
        for(; i < FILEDATA_NAME_LEN - 1 && !(_->full_name[i] == '\r' || _->full_name[i] == '\n' || _->full_name[i] == '\0'); i++);
        _->full_name[i] = '\0';
        _->full_name[0] = i - 1;
    }
    return 1;
}

void element_zerofill(ListElement * elem){
    memset(elem, 0, sizeof(ListElement));
    elem->data.gender = 'm';
    elem->data.education_form = 'd';
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

size_t get_file_size(FILE * f){
	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	rewind(f);
	return file_size;
}

void list_autoload(){
	FILE * file = fopen(AUTOSAVE_FILE_NAME, "rb");

	if(file){
				
		size_t file_size = get_file_size(file);
		//TODO: needs UI
        if(file_size % sizeof(FileData) != 0)
			puts(AUTOSAVE_FILE_NAME " seems to be corrupted");
		file_size /= sizeof(FileData);

		first_alloc_begin = malloc((file_size + 1) * sizeof(ListElement));
		first_alloc_end = first_alloc_begin + file_size;
		last_readed = first_alloc_begin;
		while(fread(last_readed, sizeof(FileData), 1, file)){
			list_add(last_readed);
			last_readed++;
		}

        fclose(file);

	} else {
		first_alloc_begin = last_readed = new(ListElement);
		first_alloc_end = first_alloc_begin + 1;
	}

}

void list_autosave(){
    FILE * file = fopen(AUTOSAVE_FILE_NAME, "wb");
	//TODO: needs UI
    if(!file){
        puts("can't open " AUTOSAVE_FILE_NAME " for writing");
		return;
	}

	for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
		if(!fwrite(&cur->data, sizeof(FileData), 1, file)){
            //TODO: needs UI
			puts("can't write to file");
			goto exit;
		}

exit:
	fflush(file);
	fclose(file);
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
        list_element_free(el);
}

CompareFunc list_element_compare_func;
size_t field_to_compare_by_offset;
int list_element_compare(ListElement * a, ListElement * b){
    return list_element_compare_func((char *) a + field_to_compare_by_offset, (char *) b + field_to_compare_by_offset);
}

typedef struct {
    ListElement * first, * last;
    ListElement * next; // указатель на начало следующего отрезка
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
        min = list_element_compare(cur_[0], cur_[1]) >= 0;
        connect(cur, cur_[min]);
        cur = cur_[min];
        if(cur == l[min].last){
            /*
            Q: why doesn't it fails when cur_[!min] == NULL
            A: cur_[!min] is never null
            A: Потому что когда хоть в одном из списков встречается последний элемент...
            В эту функцию всегда передаются два отрезка, каждый из которых в длинну > 3
            При этом первый может быть короче второго на 1 элемент, но не наоборот
            И второй отрезок идёт после первого в исходном списке
            A: Но, да если вызвать функцию отдельно с некорректными данными, она может упасть
            */
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

            connect(cut.first, cut.last);
        }
        return cut;
    }
}

//WARN: list_len should be >= 2
void merge_sort(unsigned char field_id){
    Field field = list_element_fields[field_id];
    field_to_compare_by_offset = field.offset;
    list_element_compare_func = field.comp_func;
    
    //TODO: comment back? XD
    //TODO: wrap into func?
    if((char *) field.read_func == (char *) read_string)
        field_to_compare_by_offset++;
    
    Cut cut = recursion(HEAD, list_len);
    HEAD = cut.first;
    TAIL = cut.last;
    HEAD->PREV = NULL;
    TAIL->NEXT = NULL;
}

void list_free(){
    link_layer = SHOW;
    freeded_elements = heads[SHOW]; //TODO: test
    heads[SHOW] = heads[SEARCH] = NULL;
    tails[SHOW] = tails[SEARCH] = NULL;
    list_len = 0;
}

void list_release_memory(){
    #define link_layer SHOW
    if(TAIL){
        // freeded_elements = HEAD -> freeded_elements
        // becouse we cannot connect anything to the untracked end of freeded_elements
	    TAIL->NEXT = freeded_elements;
        freeded_elements = HEAD;
    }
    // last_readed -> freeded_elements
    last_readed->NEXT = freeded_elements;
    for(ListElement * cur = last_readed; cur;){
		ListElement * next = cur->NEXT;
		if(cur <= first_alloc_begin && cur > first_alloc_end)
            free(cur);
        cur = next;
	}
	/*
	first_alloc_begin = first_alloc_end = 0;
	freeded_elements = last_readed = NULL;
    heads[SHOW] = heads[SEARCH] = NULL;
    tails[SHOW] = tails[SEARCH] = NULL;
    list_len = 0;
	*/
    #undef link_layer
}

void list_copy_to_search_layer(){
    heads[SEARCH] = heads[SHOW];
    tails[SEARCH] = tails[SHOW];
    for(ListElement * cur = heads[SHOW]; cur; cur = cur->NEXT){
        cur->link[1] = cur->link[0];
        cur->link[3] = cur->link[2];
    }
    list_lengths[SEARCH] = list_lengths[SHOW];
}

int list_process_cmp(FileData * a, FileData * b){
    int ret = strcmp(a->group_name, b->group_name);
    if(!ret){
        ret = a->gender - b->gender;
        if(!ret){
            ret = b->USE_score - a->USE_score;
            if(!ret){
                ret = (
                    a->admission_date.y * 365 + a->admission_date.m * 31 + a->admission_date.m
                ) - (
                    b->admission_date.y * 365 + b->admission_date.m * 31 + b->admission_date.m
                );
            }
        }
    }
    return ret;
}

void list_process(){
    //assert(list_len > 2)

    list_copy_to_search_layer();
    link_layer = SEARCH;

    field_to_compare_by_offset = offsetof(ListElement, data);
    list_element_compare_func = list_process_cmp;
    Cut cut = recursion(HEAD, list_len);
    HEAD = TAIL = cut.first;
    cut.last->NEXT = NULL;
    HEAD->PREV = NULL;
    list_len = 1;
    
    char i = 1;
    ListElement * cur = HEAD->NEXT;
    while(cur){
        if(i < 5){
            connect(TAIL, cur);
            TAIL = cur;
            list_len++;
            i++;
        }
        ListElement * next = cur->NEXT;
        if(next && (strcmp(next->data.group_name, cur->data.group_name) || next->data.gender != cur->data.gender))
            i = 0;
		cur = next;
    }
    
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
