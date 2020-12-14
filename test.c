#define LINUX
#include "list.h"

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
