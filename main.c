#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <locale.h>
#include <stddef.h>
#include <windows.h> 

//#include <string.h>

#include "quotes.h"

// DEFINES
#define true 1
#define false 0
#define loop while(true)
#define new(T) malloc(sizeof(T))
#define len(x)  (sizeof(x) / sizeof((x)[0]))
#define foreach(i, array) for(size_t i = 0; i < len(array); i++)
#define isspace(c) (c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')

// TYPEDEFS
typedef char bool;
//typedef char byte;

typedef struct {
    char group_name[6 * 4 + 1];
    int gradebook_number; //TODO: change to short
    char full_name[80];
    char gender;
    char education_form;
    int birth_date[3], admission_date[3];
    int USE_score;  //TODO: change to short
} FileData;

typedef struct list_element {
    FileData data;
    struct list_element * next;
} ListElement;

typedef struct {
	char shortcut;
	char * name;
	char type;
	size_t offset;
	byte size;
	byte len;
} Field;

// GLOBAL VARIABLES
#define field_len(s, f) sizeof(((s *) 0)->f)
#define field(s, g, t, o, l) { s, g, t, offsetof(FileData, o), field_len(FileData, o), l }
Field fields[] = {
	field('c', "���� ������", 's', group_name, 6),
	field('i', "����� �������� ������", 'i', gradebook_number, 0),
	field('n', "���", 's', full_name, 80),
	field('g', "���", 'c', gender, 1),
	field('f', "����� ��������", 'c', education_form, 1),
	field('b', "���� ��������", 'd', birth_date, 0),
	field('e', "���� �����������", 'd', admission_date, 0),
	field('s', "���� ���", 'i', USE_score, 0)
};
#undef field

ListElement * head, * tail;
ListElement * last_readed;
ListElement * selected, * selected_prev;

// FUNCTIONS

bool ignore(){
	if(getchar() != '\n'){
		puts("���������� ����� ����� ���������������");
		mistake_quote();
		while(getchar() != '\n');
	}
	return true;
}

void _gets(char * buf, int size, int len){
	loop {
		fgets(buf, size, stdin);
        return;
	}
}

bool read_field(int i, FileData * data_ptr){
	
	#define _ ((byte *) data_ptr)
	bool ret = true;

	if(fields[i].type == 'd'){
		int * field = (int * ) (_ + fields[i].offset);
		if(!scanf(" %i %i %i", field, field + 1, field + 2))
			ret = false;
		ignore();
	}
	else if(fields[i].type == 's'){
		_gets(_ + fields[i].offset, fields[i].size, fields[i].len);
	} else {
		char fmt[] = { ' ', '%', fields[i].type, '\0' };
		if(!scanf(fmt, _ + fields[i].offset))
			ret = false;
		ignore();
	}

	return ret;
	#undef _
}

bool read(){
	FileData * data_ptr = &last_readed->data;
	foreach(i, fields){
		if(fields[i].len)
			printf("%s [%i]: ", fields[i].name, fields[i].len);
		else
			printf("%s: ", fields[i].name);
		if(!read_field(i, data_ptr))
			return false;
	}
	return true;
}

bool _add(){
    if(read()){
		if(!head)
			head = tail = last_readed;
		else {
			tail->next = last_readed;
			tail = last_readed;
		}
		tail->next = NULL;
        last_readed = new(ListElement);
        return true;
    }
    return false;
}

void _organize(){
    while(_add());
}

void print_element(ListElement * cur){
	FileData * _ = &cur->data;
	printf("%s %i %s %c %c %i.%i.%i %i.%i.%i %i\n",
		_->group_name, _->gradebook_number, _->full_name, _->gender, _->education_form,
		_->birth_date[0],     _->birth_date[1],     _->birth_date[2],
		_->admission_date[0], _->admission_date[1], _->admission_date[2],
		_->USE_score
	);
}

/*
void print_element(ListElement * cur){
	char * _ = (char *) &cur->data;
	foreach(i, fields){
		if(fields[i].type == 'd'){
			int * field = (int * )(_ + fields[i].offset);
			printf("%i.%i.%i ", *field, *(field + 1), *(field + 2));
		} else if(fields[i].type == 's'){
			printf("%s ", _ + fields[i].offset);
		} else {
			char fmt[4] = { '%', fields[i].type, ' ', '\0' };
			printf(fmt, *(_ + fields[i].offset));
		}
	}
	putchar('\n');
}
*/

void _print(){

	if(selected){
		puts("������:");
		print_element(selected);
	} else
		puts("������� �� ������");

	if(head){
		if(selected)
			puts("������:");
		ListElement * cur = head;
		do {
			print_element(cur);
			cur = cur->next;
		} while(cur);
	} else
		puts("������ ����");
}

void _select(){
	int id;
	printf("����� �������� ������: "); scanf("%i", &id); ignore();
	ListElement * cur = head, * prev = NULL;
	while(cur){
		if(cur->data.gradebook_number == id){
			selected_prev = prev;
			selected = cur;
			puts("������:");
			print_element(selected);
			return;
		}
		prev = cur;
		cur = cur->next;
	}
	puts("Error 404. Student not found");
	mistake_quote();
	//selected_prev = NULL;
	selected = NULL;
}

void _edit(){
	if(selected){
		puts("���� ��� ��������������:");
		foreach(i, fields)
			printf("%c %s\n", fields[i].shortcut, fields[i].name);
		char ch;
		putchar('>'); scanf(" %c", &ch); ignore();
		foreach(i, fields)
			if(fields[i].shortcut == ch){
				printf("����� �������� ����: ");
				read_field(i, &selected->data);
				//puts("���������������");
				edit_quote();
				return;
			}
		puts("Error 404. ���� �� �������");
		mistake_quote();
	} else {
		puts("Error 400. ������ �� �������");
		mistake_quote();
	}
}

void _remove(){
	if(selected){
		if(selected == head){
			head = selected->next;
		} else {
			selected_prev->next = selected->next;
			if(selected == tail)
				tail = selected_prev;
		}
		free(selected);
		selected_prev = NULL;
		selected = NULL;
		//puts("�������");
		remove_quote();
	} else {
		puts("Error 400. ������ �� �������");
		mistake_quote();
	}
}

int main(){

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cursor_pos = { 0 ,0 };

	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
	//FillConsoleOutputAttribute(hConsole, );
	
	setlocale(LC_ALL, "rus");

	FILE * internal = fopen("internal", "r+");
	if(internal)
		fread(&quotes_state, sizeof(QuotesState), 1, internal);
	else
		internal = fopen("internal", "w+");

	start_quote();

	head = tail = NULL;
	selected = NULL;
    last_readed = new(ListElement);

	while(true){

		SetConsoleCursorPosition(hConsole, cursor_pos);
		
		putchar('>');
        char ch = getch();
		//scanf(" %c", &ch); ignore();
		printf("%c\n", ch);
        switch(ch){
			case 'o': _organize();	break;
			case 'a': _add();		break;
			case 'p': _print(); 	break;
			case 's': _select();	break;
			case 'e': _edit();		break;
			case 'r': _remove();	break;
			case 'q': goto exit;
			default:
				puts("Error 404. ������� �� �������");
				mistake_quote();
		}
	}

exit:;
	ListElement * tmp, * cur = head;
	while(cur){
		tmp = cur;
		cur = cur->next;
		free(tmp);
	}
    free(last_readed);

	exit_quote();
	
	fseek(internal, 0, SEEK_SET);
	fwrite(&quotes_state, sizeof(QuotesState), 1, internal);
	fclose(internal);

    return 0;
}
