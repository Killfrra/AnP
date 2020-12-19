#define LINUX
#include "list.h"

int main(){
	ListElement _last_readed;
	last_readed = &_last_readed;

	FILE * file = fopen("test.txt", "r");
    if(!file){
        puts("can't open test.txt for reading");
		return 1;
	}
	
	link_layer = SHOW;
	HEAD = TAIL = NULL;
	list_len = 0;

    while(element_read_from_txt(file, last_readed) >= 0){
		list_add(last_readed);
		last_readed = new(ListElement);
	}

	fclose(file);
	
	//list_autosave();
	//list_autoload();

	for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
		element_print(cur);

	list_release_memory();
	return 0;
}
