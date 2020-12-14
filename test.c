#define LINUX
#include "list.h"

/*
void menu_process(){
    list_copy_to_search_layer();
    link_layer = SEARCH;
    merge_sort(0); // indexof(group) = 0
    struct {
        ListElement * link;
        unsigned short score;
    } excellents[5] = { 0 };
    
    ListElement * cur = HEAD;
    HEAD = TAIL = NULL;
    list_len = 0;
    while(cur){
        char inserted = FALSE;
        unsigned short score = cur->data.USE_score;
        for(char i = 0; i < 5; i++)
            if(excellents[i].score < score){
                char last_better_than_penultimate = excellents[4].link && excellents[3].score == excellents[4].score && intcmp(&excellents[3].link->data.admission_date, &excellents[4].link->data.admission_date) < 0;
				memmove(&excellents[i + 1], &excellents[i], sizeof(excellents[0]) * (5 - i - 1 - last_better_than_penultimate));
                excellents[i].score = score;
                excellents[i].link = cur;
                inserted = TRUE;
                break;
            }
        if(!inserted && score == excellents[4].score && intcmp(&cur->data.admission_date, &excellents[4].link->data.admission_date) < 0){ //TODO: datecmp
            excellents[4].score = score;
            excellents[4].link = cur;
        }
        ListElement * next = cur->NEXT;
        if(!next || strcmp(&next->data.group_name[1], &cur->data.group_name[1])){
            char i = 0;
            if(!HEAD){
                HEAD = TAIL = excellents[0].link;
                HEAD->PREV = NULL;
                list_len = i = 1;
            }
            for(; i < 5 && excellents[i].link; i++){
				connect(TAIL, excellents[i].link);
				TAIL = excellents[i].link;
            }
            memset(excellents, 0, sizeof(excellents));
        }
		cur = next;
    }
    if(TAIL)
        TAIL->NEXT = NULL;
}
*/

int main(){
	ListElement * last_readed = new(ListElement);

	FILE * file = fopen("test.txt", "r");
    if(!file)
        puts("can't open the file");
	
	link_layer = SHOW;
	HEAD = TAIL = NULL;
	list_len = 0;

    while(element_read_from_txt(file, last_readed) >= 0){
		
		if(!HEAD)
			HEAD = TAIL = last_readed;
		else {
			connect(TAIL, last_readed);
			TAIL = last_readed;
		}
		list_len++;

		last_readed = new(ListElement);
	}

	//for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
	//	element_print(cur);

	list_process();
	
	// list_free
	for(ListElement * cur = HEAD; cur;){
		element_print(cur);
		ListElement * prev = cur;
		cur = cur->NEXT;
		free(prev);
	}

	free(last_readed);
	return 0;
}
