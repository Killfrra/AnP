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
    unsigned int gradebook_number; // 4
    char full_name[32];     // 32
    char gender, education_form; // 1 + 1
    Date birth_date, admission_date; // 4 + 4
    unsigned short USE_score; // 2
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
	{ 'c', "Шифр группы", 's', offsetof(FileData, group_name), 6, { allow_digits: TRUE }},
	{ 'i', "Номер зачетной книжки", 'i', offsetof(FileData, gradebook_number), 6 },
	{ 'g', "Пол", 'c', offsetof(FileData, gender), 1, { values: "\2mf" } },
	{ 'f', "Форма обучения", 'c', offsetof(FileData, education_form), 1, { values: "\3ozd" } },
	{ 'b', "Дата рождения", 'd', offsetof(FileData, birth_date), 10 },
	{ 'e', "Дата поступления", 'd', offsetof(FileData, admission_date), 10 },
	{ 's', "Балл ЕГЭ", 'h', offsetof(FileData, USE_score), 3},
    { 'n', "ФИО", 's', offsetof(FileData, full_name), sizeof(((FileData*) 0)->full_name), { allow_digits: FALSE } }
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
	printf("%-6s %06u %c %c ", &_->group_name[1], _->gradebook_number, _->gender, _->education_form);
    print_date(_->birth_date);
    putchar(' ');
    print_date(_->admission_date);
    printf(" %03hu %s", _->USE_score, &_->full_name[1]);
}

ListElement * new_empty_element(){
    ListElement * ret = calloc(1, sizeof(ListElement));
    ret->data.gender = ret->data.education_form = ' ';
    return ret;
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
    
    } else {
        clear_lines(list_first_line, list_last_line);
        puts("\n Nothing to show");
    }
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


#define POSY 2 //TODO: remove
void edit_element(ListElement * elem); //TODO: перетасовать функции и удалить
void add_element(){
    setCursorPosition(0, 1);
    /*
    #define header "\\\\Adding element\\\\"
    #define header_len (len(header) - 1)
    printf(header);
    repeat(header_len, 1, buffer_info.dwSize.X - header_len, '_');
    #undef header_len
    #undef header
    */
    clear_lines(1, 2);
    setCursorPosition(0, POSY);
    print_element(last_readed);
    repeat(0, 3, buffer_info.dwSize.X, '_');
    setCursorVisibility(TRUE);
    edit_element(last_readed);
    setCursorVisibility(FALSE);
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
    SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
}

void draw_menu(){
    setColor(0, 0, buffer_info.dwSize.X, BACKGROUND_BLUE);
    redraw_menu();
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

    setCursorPosition(posx, POSY);
    printf("%s", buffer);
    setCursorPosition(posx + cursor_pos, POSY);

    int ch;
    loop {
        
        ch = _getch();
        //printf("DEBUG: %d\n", ch);
        if(ch == 224){
            ch = _getch();
            //printf("DEBUG: %d\n", ch);
            if(ch == ARROW_LEFT){
                if(cursor_pos != 0){
                    setCursorPosition(posx + cursor_pos - 1, POSY);
                    cursor_pos--;
                } else
                    goto exit;
            } else if(ch == ARROW_RIGHT){
                if(cursor_pos != last_char){
                    setCursorPosition(posx + cursor_pos + 1, POSY);
                    cursor_pos++;
                } else
                    goto exit;
            } else if(ch == ARROW_UP){
                cursor_pos = 0;
                setCursorPosition(posx, POSY);
            } else if(ch == ARROW_DOWN){
                cursor_pos = last_char;
                setCursorPosition(posx + cursor_pos, POSY);
            }
        } else {
            //printf("DEBUG: %c not 244\n", ch);
            if(ch == KEY_ENTER)
                goto exit;
            else if(ch == KEY_BACKSPACE){
                if(cursor_pos != 0){
                    setCursorPosition(posx + cursor_pos - 1, POSY);
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
                    setCursorPosition(posx + cursor_pos, POSY);
                }
            } else if
            (
                (allow_digits && ch >= '0' && ch <= '9') ||
                (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'а' && ch <= 'я') || (ch >= 'А' && ch <= 'Я') ||
                // пробел не может быть первым, не может идти два раза подряд
                (ch == ' ' && (cursor_pos != 0 && buffer[cursor_pos - 1] != ' '))
            ){

                if(ch == ' ' && buffer[cursor_pos] == ' '){
                    setCursorPosition(posx + (++cursor_pos), POSY);
                    continue;
                }

                //TODO: think: оставлять ли последний char в buffer под '\0'?
                if(last_char == buffer_size){
                    setCursorPosition(posx, POSY + 1);
                    puts("Too many letters!");
                    setCursorPosition(posx + cursor_pos, POSY);
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

                setCursorPosition(posx + cursor_pos, POSY);
            }
        }
    }
exit:
    if(buffer[last_char - 1] == ' ')
        last_char--;
    buffer[last_char] = '\0';
    dest[0] = last_char;
    return ch;
}

char read_fixed_int(char enter_dir, short posx, unsigned int * dest, unsigned char n_digits){
    
    char buffer[11];
    unsigned char cursor_pos = 0;
    if(enter_dir == ARROW_LEFT)
        cursor_pos = n_digits - 1;

    int temp = *dest;
    for(int i = n_digits - 1; i >= 0; i--){
        buffer[i] = '0' + (temp % 10);
        temp /= 10;
    }
    buffer[n_digits] = '\0';

    setCursorPosition(posx, POSY);
    printf("%s", buffer);
    setCursorPosition(posx + cursor_pos, POSY);

    int ch;
    loop {
        ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_LEFT){
                if(cursor_pos != 0){
                    cursor_pos--;
                    setCursorPosition(posx + cursor_pos, POSY);
                } else
                    goto exit;
            } else if(ch == ARROW_RIGHT){
                if(cursor_pos != n_digits - 1){
                    cursor_pos++;
                    setCursorPosition(posx + cursor_pos, POSY);
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

                setCursorPosition(posx + cursor_pos, POSY);
            }
        }
    }

exit:
    *dest = atoi(buffer);
    return ch;
}

char read_fixed_short(char enter_dir, short posx, unsigned short * dest, unsigned char n_digits){
    unsigned int temp = *dest;
    char ret = read_fixed_int(enter_dir, posx, &temp, n_digits);
    *dest = temp & 0xFFFF;
    return ret;
}

char read_fixed_date(char enter_dir, short posx, Date * dest){
    
    char buffer[11] = {
        '0' + (dest->d / 10) % 10,
        '0' + (dest->d % 10),
        '.',
        '0' + (dest->m / 10) % 10,
        '0' + (dest->m) % 10,
        '.',
        '0' + (dest->y / 1000) % 10,
        '0' + (dest->y / 100) % 10,
        '0' + (dest->y / 10) % 10,
        '0' + (dest->y % 10),
        '\0'
    };

    unsigned char cursor_pos = 0;
    if(enter_dir == ARROW_LEFT)
        cursor_pos = 9;

    setCursorPosition(posx, POSY);
    puts(buffer);
    setCursorPosition(posx + cursor_pos, POSY);

    #define inc_cursor_pos() { \
        cursor_pos += 1 + (cursor_pos == 1 || cursor_pos == 4); \
        setCursorPosition(posx + cursor_pos, POSY); \
    }
    #define dec_cursor_pos() { \
        cursor_pos -= 1 + (cursor_pos == 3 || cursor_pos == 6); \
        setCursorPosition(posx + cursor_pos, POSY); \
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
    dest->d = (buffer[0] - '0') * 10 + (buffer[1] - '0');
    dest->m = (buffer[3] - '0') * 10 + (buffer[4] - '0');
    dest->y = (buffer[6] - '0') * 1000 + (buffer[7] - '0') * 100 +
              (buffer[8] - '0') * 10   + (buffer[9] - '0');
    return ch;
}

char read_char(short posx, char * dest, char * values){
    char n_values = (values++)[0];
    char buffer = *dest;
    
    setCursorPosition(posx, POSY);
    putchar(buffer);
    setCursorPosition(posx, POSY);

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

void edit_element(ListElement * elem){
    unsigned char current_field = 0;
    char ret = ARROW_RIGHT;
    char posx = 0;
    loop {
        Field field = form[current_field];
        //printf("\n%c%d", field.shortcut, posx);
        char * offset = (char * ) elem + field.offset;
        if(field.type == 'c')
            ret = read_char(posx, offset, field.prop.values);
        if(field.type == 's')
            ret = read_string(ret, posx, offset, field.size, field.prop.allow_digits);
        else if(field.type == 'i')
            ret = read_fixed_int(ret, posx, (unsigned int*) offset, field.size);
        else if(field.type == 'h')
            ret = read_fixed_short(ret, posx, (unsigned short*) offset, field.size);
        else if(field.type == 'd')
            ret = read_fixed_date(ret, posx, (Date*) offset);

        if(ret == ARROW_LEFT){
            if(current_field != 0){
                current_field--;
                posx -= form[current_field].size + 1;
            }
        } else if(current_field == len(form) - 1){
            break;
        } else {
            posx += form[current_field].size + 1;
            current_field++;
        }
    }
}

int main(){

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(stdout_handle, &cursor_info);
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
    
    ListElement temp = {
        data: {
            gradebook_number: 873310,
            gender: ' ',
            education_form: ' ',
            full_name: "\x7testers",
            birth_date: { 16, 11, 2001 }
        }
    };
    last_readed = &temp;
    
    /*
    for(char i = 0; i < len(menu_items); i++){
        menu_len += menu_items[i].name_len + 2; // + 2 spaces
    }
    *//*
    add("one"); // one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one");
    add("two"); // two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two");
    add("three"); // three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three");
    add("four"); // four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four");
    add("five"); // five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five");
    */
    
    setCursorVisibility(FALSE);
    clear_lines(0, buffer_info.dwSize.Y);
      draw_menu();
    redraw_list();

    /*
    setCursorPosition(0, 1);
    print_element(last_readed);
    
    edit_element(last_readed);
    
    setCursorPosition(0, 1);
    print_element(last_readed);
    */
    
    loop {
        int ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_UP)
                scroll_list(UP);
            else if(ch == ARROW_DOWN)
                scroll_list(DOWN);
            else if(ch == ARROW_LEFT)
                scroll_menu(LEFT);
            else if(ch == ARROW_RIGHT)
                scroll_menu(RIGHT);
        } else if(ch == KEY_ENTER)
            menu_items[selected_menu_item].func();
        else
            break;
    }

    clear_lines(0, buffer_info.dwSize.Y);
    setCursorVisibility(TRUE);

    return 0;
}