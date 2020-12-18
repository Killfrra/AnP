#define LINUX
#include "list.h"

int main(){

	
	/*
	file = fopen("test.txt", "r");
    if(!file){
        puts("can't open test.txt for reading");
		return 1;
	}
	
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

	fclose(file);
	
	list_autosave();
	*/

	list_autoload();

	for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
		element_print(cur);

	list_release_memory();
	return 0;
}
