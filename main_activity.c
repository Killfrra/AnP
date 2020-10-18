#include <stdio.h>
#include <windows.h>
#include <conio.h>
//#include <string.h>
#include <stddef.h>

#define loop while(1)
#define new(T) malloc(sizeof(T))
#define len(x)  (sizeof(x) / sizeof((x)[0]))

#define ARROW_UP 72
#define ARROW_DOWN 80
#define ARROW_LEFT 75
#define ARROW_RIGHT 77
#define KEY_ENTER 13
#define KEY_BACKSPACE 8

#define BACKGROUND_WHITE (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)

HANDLE stdout_handle;
void setCursorPosition(SHORT x, SHORT y){
    COORD coord = { x, y };
    SetConsoleCursorPosition(stdout_handle, coord);
}

CONSOLE_CURSOR_INFO cursor_info;
void setCursorVisibility(char state){
    cursor_info.bVisible = state;
    SetConsoleCursorInfo(stdout_handle, &cursor_info);
}

CONSOLE_SCREEN_BUFFER_INFO buffer_info;
void clear_lines(short from, short to){
    COORD coord = { 0, from };
    DWORD written;
    FillConsoleOutputCharacter(stdout_handle, ' ', buffer_info.dwSize.X * to, coord, &written);
    FillConsoleOutputAttribute(stdout_handle, buffer_info.wAttributes, buffer_info.dwSize.X * to, coord, &written);
    setCursorPosition(0, from);
}

void setColor(short from_x, short from_y, DWORD _len, WORD attr){
    DWORD written;
    COORD coord = { from_x, from_y };
    FillConsoleOutputAttribute(stdout_handle, attr, _len, coord, &written);
}

void repeat(short from_x, short from_y, DWORD _len, char c){
    DWORD written;
    COORD coord = { from_x, from_y };
    FillConsoleOutputCharacter(stdout_handle, c, _len, coord, &written);
}

typedef struct {
    unsigned char d, m;
    unsigned short y;
} Date;

void print_date(Date date){
    printf("%02hu.%02hu.%04hu", (unsigned short) date.d, (unsigned short) date.m, (unsigned short) date.y);
}

typedef struct {
    char group_name[6 + 1]; // 7
    int gradebook_number; // 4
    char full_name[32];     // 32
    char gender, education_form; // 1 + 1
    Date birth_date, admission_date; // 4 + 4
    int USE_score;    // TODO: change to short
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

Field form[] = {
	{ 'c', "���� ������", 's', offsetof(FileData, group_name), 6, { allow_digits: TRUE }},
	{ 'i', "����� �������� ������", 'i', offsetof(FileData, gradebook_number), 6 },
	{ 'n', "���", 's', offsetof(FileData, full_name), sizeof(((FileData*) 0)->full_name), { allow_digits: FALSE } },
	{ 'g', "���", 'c', offsetof(FileData, gender), 1, { values: "\2mf" } },
	{ 'f', "����� ��������", 'c', offsetof(FileData, education_form), 1, { values: "\3ozd" } },
	{ 'b', "���� ��������", 'd', offsetof(FileData, birth_date), 10 },
	{ 'e', "���� �����������", 'd', offsetof(FileData, admission_date), 10 },
	{ 's', "���� ���", 'i', offsetof(FileData, USE_score), 3}
};

ListElement * head = NULL;
ListElement * tail = NULL;
ListElement * first_element_on_screen = NULL;
ListElement * selected_element = NULL;
ListElement * last_readed;

int selected_element_pos_on_screen = -1;
short list_first_line = 1;
short list_last_line = 5;

/*
void add(char * name){
    ListElement * el = new(ListElement);
    //el->data.name = name;
    strcpy(el->data.name, name);
    if(!head){
        el->PREV = NULL;
        head = el;

        first_element_on_screen = head;
        selected_element = head;
        selected_element_pos_on_screen = 0;

    } else {
        tail->NEXT = el;
        el->PREV = tail;
    }
    el->NEXT = NULL;
    tail = el;
}
*/
void print_element(ListElement * cur){
	FileData * _ = &cur->data;
	printf("%s %hu %s %c %c ", &_->group_name[1], _->gradebook_number, &_->full_name[1], _->gender, _->education_form);
    print_date(_->birth_date);
    putchar(' ');
    print_date(_->admission_date);
    printf(" %hu", _->USE_score);
}

void redraw_list(){

    setCursorPosition(0, list_first_line); //clear_lines(0, BUFFER_Y);

    if(head){
        ListElement * cur = first_element_on_screen;
        for(int i = 0; cur && i < (list_last_line - list_first_line); i++){
            if(i == selected_element_pos_on_screen){
                SetConsoleTextAttribute(stdout_handle, BACKGROUND_WHITE);
                print_element(cur);
                SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
            } else
                print_element(cur);
            cur = cur->NEXT;
        }
        setColor(0, selected_element_pos_on_screen + list_first_line, buffer_info.dwSize.X - 1, BACKGROUND_WHITE);
        
        setCursorPosition(buffer_info.dwSize.X - 1, list_first_line);
        if(first_element_on_screen != head)
            putchar('^');
        else
            putchar(' ');

        setCursorPosition(buffer_info.dwSize.X - 1, list_last_line);
        if(cur)
            putchar('v');
        else
            putchar(' ');
    
    } else
        puts("\n Nothing to show");
}

typedef enum { UP, DOWN } Vertical;
void scroll_list(Vertical dir){
    if(!head) return;
    if(selected_element->DIR(dir)){

        if(dir == UP && selected_element_pos_on_screen == 0){
            first_element_on_screen = first_element_on_screen->PREV;
            selected_element = first_element_on_screen;
            redraw_list();
        } else if(dir == DOWN && selected_element_pos_on_screen == list_last_line - list_first_line - 1){
            first_element_on_screen = first_element_on_screen->NEXT;
            selected_element = selected_element->NEXT;
            redraw_list();
        } else {

            setColor(0, selected_element_pos_on_screen + list_first_line, buffer_info.dwSize.X - 1, buffer_info.wAttributes);

            // char offset[2] = { -1, 1 };
            selected_element_pos_on_screen += dir * 2 - 1; //offset[dir];
            selected_element = selected_element->DIR(dir);

            setColor(0, selected_element_pos_on_screen + list_first_line, buffer_info.dwSize.X - 1, BACKGROUND_WHITE);
        }
    } //else
      //  Beep(750, 100);
}

typedef struct {
    char * name;
    char   name_len;
    void (* func)();
} MenuItem;

void empty(){
    printf("DEBUG: empty called");
}

void add_element(){
    setCursorPosition(0, 1);
    printf("Adding element");
    #define _len len("Adding element")
    repeat(_len, 1, buffer_info.dwSize.X - _len, '-');
    #undef _len
    clear_lines(2, 2);
    repeat(0, 3, buffer_info.dwSize.X, '-');
    _getch();
    redraw_list();
}

#define item(name, func) { name, len(name) - 1, func }
MenuItem menu_items[] = {
    item("+",  add_element ),
    item("-",  empty),
    item("Edit",  empty),
    item("Search", empty),
    item("Sort", empty),
    item("Export", empty),
    item("Import", empty),
    item("Save", empty),
    item("Process", empty)
};

signed char selected_menu_item = 0;
int menu_len = 0;
void redraw_menu(){
    setCursorPosition( 0, 0);
    SetConsoleTextAttribute(stdout_handle, BACKGROUND_BLUE);
    for(unsigned char i = 0; i < len(menu_items); i++){
        if(i == selected_menu_item){
            SetConsoleTextAttribute(stdout_handle, BACKGROUND_GREEN);
            printf(" %s ", menu_items[i].name);
            SetConsoleTextAttribute(stdout_handle, BACKGROUND_BLUE);
        } else
            printf(" %s ", menu_items[i].name);
    }
    COORD coord = { menu_len, 0 };
    DWORD written;
    FillConsoleOutputAttribute(stdout_handle, BACKGROUND_BLUE, buffer_info.dwSize.X - menu_len, coord, &written);
    SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
}

typedef enum { LEFT, RIGHT } Horizontal;
void scroll_menu(Horizontal dir){

    selected_menu_item += dir * 2 - 1;
    if(selected_menu_item < 0)
        selected_menu_item = len(menu_items) - 1;
    else if(selected_menu_item == len(menu_items))
        selected_menu_item = 0;
    
    redraw_menu();
}

char read_string(char enter_dir, short posx, char * dest, char buffer_size, char allow_digits){
    
    char * buffer = &dest[1];
    unsigned char last_char = dest[0];
    unsigned char cursor_pos = 0;
    if(enter_dir == ARROW_LEFT)
        cursor_pos = last_char;

    setCursorPosition(posx, 0);
    printf("%s", buffer);
    setCursorPosition(posx + cursor_pos, 0);

    int ch;
    loop {
        
        ch = _getch();
        //printf("DEBUG: %d\n", ch);
        if(ch == 224){
            ch = _getch();
            //printf("DEBUG: %d\n", ch);
            if(ch == ARROW_LEFT){
                if(cursor_pos != 0){
                    setCursorPosition(posx + cursor_pos - 1, 0);
                    cursor_pos--;
                } else
                    goto exit;
            } else if(ch == ARROW_RIGHT){
                if(cursor_pos != last_char){
                    setCursorPosition(posx + cursor_pos + 1, 0);
                    cursor_pos++;
                } else
                    goto exit;
            } else if(ch == ARROW_UP){
                cursor_pos = 0;
                setCursorPosition(posx, 0);
            } else if(ch == ARROW_DOWN){
                cursor_pos = last_char;
                setCursorPosition(posx + cursor_pos, 0);
            }
        } else {
            //printf("DEBUG: %c not 244\n", ch);
            if(ch == KEY_ENTER)
                goto exit;
            else if(ch == KEY_BACKSPACE){
                if(cursor_pos != 0){
                    setCursorPosition(posx + cursor_pos - 1, 0);
                    if(cursor_pos == last_char)
                        putchar(' ');
                    else {
                        for(int i = cursor_pos - 1; i < last_char - 1; i++)
                            buffer[i] = buffer[i + 1];
                        buffer[last_char - 1] = '\0';
                        printf("%s ", &buffer[cursor_pos - 1]);
                    }
                    cursor_pos--;
                    last_char--;
                    setCursorPosition(posx + cursor_pos, 0);
                }
            } else if
            (
                (allow_digits && ch >= '0' && ch <= '9') ||
                (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                (ch >= '�' && ch <= '�') || (ch >= '�' && ch <= '�') ||
                // ������ �� ����� ���� ������, �� ����� ���� ��� ���� ������
                (ch == ' ' && (cursor_pos != 0 && buffer[cursor_pos - 1] != ' '))
            ){

                if(ch == ' ' && buffer[cursor_pos] == ' '){
                    setCursorPosition(posx + (++cursor_pos), 0);
                    continue;
                }

                //TODO: think: ��������� �� ��������� char � buffer ��� '\0'?
                if(last_char == buffer_size){
                    setCursorPosition(posx, 1);
                    puts("Too many letters!");
                    setCursorPosition(posx + cursor_pos, 0);
                    continue;
                }

                if(cursor_pos == last_char){
                    buffer[cursor_pos] = ch;
                    putchar(ch);
                } else {
                    for(int i = last_char; i > cursor_pos; i--)
                        buffer[i] = buffer[i - 1];
                    buffer[cursor_pos] = ch;
                    printf("%s", &buffer[cursor_pos]);
                }
                cursor_pos++;
                last_char++;

                setCursorPosition(posx + cursor_pos, 0);
            }
        }
    }
exit:
    if(buffer[last_char - 1] == ' ')
        buffer[--last_char] = '\0';
    dest[0] = last_char;
    return ch;
}

char read_fixed_int(char enter_dir, short posx, int * dest, char n_digits){
    
    #define buffer_size      9
    #define buffer_size_str "9"
    #define format "d"

    char real_buffer[buffer_size + 1];
    char * buffer;
    unsigned char cursor_pos = 0;
    if(enter_dir == ARROW_LEFT)
        cursor_pos = n_digits - 1;

    sprintf(real_buffer, "%0" buffer_size_str format, *dest);
    real_buffer[buffer_size] = '\0';
    buffer = &real_buffer[buffer_size - n_digits];

    setCursorPosition(posx, 0);
    printf("%s", buffer);
    setCursorPosition(posx + cursor_pos, 0);

    int ch;
    loop {
        ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_LEFT){
                if(cursor_pos != 0){
                    cursor_pos--;
                    setCursorPosition(posx + cursor_pos, 0);
                } else
                    goto exit;
            } else if(ch == ARROW_RIGHT){
                if(cursor_pos != n_digits - 1){
                    cursor_pos++;
                    setCursorPosition(posx + cursor_pos, 0);
                } else
                    goto exit;
            }
        } else {
            if(ch == KEY_ENTER)
                goto exit;
            else if(ch >= '0' && ch <= '9'){
                buffer[cursor_pos] = ch;
                putchar(ch);
                cursor_pos++;

                if(cursor_pos == n_digits)
                    goto exit;

                setCursorPosition(posx + cursor_pos, 0);
            }
        }
    }

exit:
    sscanf(buffer, "%" format, dest);
    return ch;

    #undef buffer_size
    #undef buffer_size_str
    #undef format
}

char read_fixed_date(char enter_dir, short posx, Date * dest){
    
    char buffer[11];

    if(dest->d == 0)
        buffer[0] = buffer[1] = '0';
    if(dest->m == 0)
        buffer[3] = buffer[4] = '0';
    if(dest->y == 0)
        buffer[6] = buffer[7] = buffer[8] = buffer[9] = '0';
    buffer[2] = buffer[5] = '.';
    buffer[10] = '\0';

    unsigned char cursor_pos = 0;
    if(enter_dir == ARROW_LEFT)
        cursor_pos = 9;

    setCursorPosition(posx, 0);
    printf("%s", buffer);
    setCursorPosition(posx + cursor_pos, 0);

    #define inc_cursor_pos() { \
        cursor_pos += 1 + (cursor_pos == 1 || cursor_pos == 4); \
        setCursorPosition(posx + cursor_pos, 0); \
    }
    #define dec_cursor_pos() { \
        cursor_pos -= 1 + (cursor_pos == 3 || cursor_pos == 6); \
        setCursorPosition(posx + cursor_pos, 0); \
    }

    int ch;
    loop {
        ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_LEFT){
                if(cursor_pos != 0){
                    dec_cursor_pos();
                } else
                    goto exit;
            } else if(ch == ARROW_RIGHT){
                if(cursor_pos != 9){
                    inc_cursor_pos();
                } else
                    goto exit;
            }
        } else {
            if(ch == KEY_ENTER)
                goto exit;
            else if(
                (cursor_pos == 0 && ch >= '0' && (ch <= '2' || (ch == '3' && buffer[1] <= '1'))) ||
                (cursor_pos == 1 && ch >= '0' && (ch == '1' || (ch <= '9' && buffer[0] != '3'))) ||
                (cursor_pos == 3 && (ch == '0' || (ch == '1' && buffer[4] <= '2'))) ||
                (cursor_pos == 4 && ch >= '0' && (ch <= '2' || (ch <= '9' && buffer[3] != '1'))) ||
                (cursor_pos >= 6 && (ch >= '0' && ch <= '9'))
            ){
                buffer[cursor_pos] = ch;
                putchar(ch);
                
                if(cursor_pos == 9)
                    goto exit;

                inc_cursor_pos();
            }
        }
    }

    #undef inc_cursor_pos
    #undef dec_cursor_pos
    
exit:
    buffer[2] = buffer[5] = '\0';
    unsigned short temp;
    sscanf(&buffer[0], "%hu", &temp); dest->d = temp;
    sscanf(&buffer[3], "%hu", &temp); dest->m = temp;
    sscanf(&buffer[6], "%hu" , &dest->y);
    return ch;
}

char read_char(short posx, char * dest, char * values){
    char n_values = (values++)[0];
    char buffer = *dest;
    
    setCursorPosition(posx, 0);
    putchar(buffer);
    setCursorPosition(posx, 0);

    int ch;
    loop {
        ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_LEFT || ch == ARROW_RIGHT)
                goto exit;
        } else {
            if(ch == KEY_ENTER)
                goto exit;
            else {
                for(unsigned char i = 0; i < n_values; i++)
                    if(ch == values[i]){
                        buffer = ch;
                        putchar(ch);
                        goto exit;
                    }
            }
        }
    }

exit:
    *dest = buffer;
    return ch;
}

int main(){

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(stdout_handle, &cursor_info);
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
    
    ListElement temp = {
        data: {
            full_name: "\x7testers",
            birth_date: {0}
        }
    };
    last_readed = &temp;
    
    /*
    for(char i = 0; i < len(menu_items); i++){
        menu_len += menu_items[i].name_len + 2; // + 2 spaces
    }
    */
    /*
    add("one"); // one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one");
    add("two"); // two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two");
    add("three"); // three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three");
    add("four"); // four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four");
    add("five"); // five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five");
    */
    //setCursorVisibility(FALSE);
    clear_lines(0, buffer_info.dwSize.Y);
    //redraw_menu();
    //redraw_list();

    setCursorPosition(0, 0);
    print_element(last_readed);
    _getch();

    unsigned char current_field = 0;
    char ret = ARROW_RIGHT;
    char posx = 0;
    loop {
        Field field = form[current_field];
        printf("\n%c%d", field.shortcut, posx);
        char * offset = (char * ) last_readed + field.offset;
        if(field.type == 'c')
            ret = read_char(posx, offset, field.prop.values);
        if(field.type == 's'){
            printf("\nDEBUG: %d %d %u %d %d", ret, posx, offset, field.size, field.prop.allow_digits);
            ret = read_string(ret, posx, offset, field.size, field.prop.allow_digits);
        } else if(field.type == 'i')
            ret = read_fixed_int(ret, posx, (int*) offset, field.size);
        else if(field.type == 'd')
            ret = read_fixed_date(ret, posx, (Date*) offset);

        if(ret == ARROW_LEFT){
            if(current_field != 0)
                current_field--;
            posx -= form[current_field].size + 1;
        } else if(current_field == len(form) - 1){
            break;
        } else {
            posx += form[current_field].size + 1;
            current_field++;
        }
    }

    //read_fixed_int(ARROW_RIGHT, 0, (unsigned short *)(((char *) last_readed) + form[1].offset), form[1].size);
    //printf("%hu %hu\n", (int *)(((char *) last_readed) + form[1].offset), &last_readed->data.gradebook_number);

    clear_lines(0, buffer_info.dwSize.Y);
    setCursorPosition(0, 0);
    print_element(last_readed);
    
    //char tmp = 'a';
    //read_char(0, &tmp, "\3ozd");
    //read_string(ARROW_RIGHT, 0, &temp.data.gender, 1);
    /*
    int tmp = 31;
    char ret = ARROW_RIGHT;
string:
    ret = read_string(ret, 14, ((char *) last_readed) + form[2].offset, form[2].size, FALSE);
date:
    ret = read_fixed_date(ret, &last_readed->data.birth_date);
    if(ret == ARROW_LEFT)
        goto string;
fixed_int:
    ret = read_fixed_int(ret, &tmp, 2);
    if(ret == ARROW_LEFT)
        goto date;

    putchar('\n');
    puts(&last_readed->data.full_name[1]);
    print_date(last_readed->data.birth_date);
    printf("\n%d\n", tmp);
    */
    /*
    loop {
        if(ch == ARROW_UP)
            scroll_list(UP);
        else if(ch == ARROW_DOWN)
            scroll_list(DOWN);
        else if(ch == ARROW_LEFT)
            scroll_menu(LEFT);
        else if(ch == ARROW_RIGHT)
            scroll_menu(RIGHT);
        else if(ch == KEY_ENTER)
            menu_items[selected_menu_item].func();
        else
            break;
    }
    */

    //clear_lines(0, buffer_info.dwSize.Y);
    //setCursorVisibility(TRUE);

    return 0;
}