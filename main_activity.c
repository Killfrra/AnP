#include <stdio.h>
#include <windows.h>

#define loop while(1)
#define new(T) malloc(sizeof(T))
#define len(x)  (sizeof(x) / sizeof((x)[0]))

#define ARROW_UP 72
#define ARROW_DOWN 80
#define ARROW_LEFT 75
#define ARROW_RIGHT 77
#define KEY_ENTER 13

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
    char * name;
} FileData;

typedef struct list_element {
    FileData data;
    struct list_element * link[4];
} ListElement;

enum { SHOW = 0, SEARCH = 2 } link_layer = SHOW;

#define DIR(d) link[d + link_layer]
#define PREV   link[0 + link_layer]
#define NEXT   link[1 + link_layer]

ListElement * head = NULL;
ListElement * tail = NULL;
ListElement * first_element_on_screen = NULL;
ListElement * selected_element = NULL;
int selected_element_pos_on_screen = -1;
short list_first_line = 1;
short list_last_line = 5;

void add(char * name){
    ListElement * el = new(ListElement);
    el->data.name = name;
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

void print(ListElement * el){
    printf("%s\n", el->data.name);
}

void redraw_list(){

    setCursorPosition(0, list_first_line); //clear_lines(0, BUFFER_Y);

    if(head){
        ListElement * cur = first_element_on_screen;
        for(int i = 0; cur && i < (list_last_line - list_first_line); i++){
            if(i == selected_element_pos_on_screen){
                SetConsoleTextAttribute(stdout_handle, BACKGROUND_WHITE);
                print(cur);
                SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
            } else
                print(cur);
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
    for(int i = 0; i < len(menu_items); i++){
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

int main(){

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(stdout_handle, &cursor_info);
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
    for(char i = 0; i < len(menu_items); i++){
        menu_len += menu_items[i].name_len + 2; // + 2 spaces
    }
    
    add("one"); // one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one");
    add("two"); // two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two");
    add("three"); // three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three");
    add("four"); // four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four");
    add("five"); // five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five");
    
    setCursorVisibility(FALSE);
    clear_lines(0, buffer_info.dwSize.Y);
    redraw_menu();
    redraw_list();
    
    loop {
        int ch = _getch();
        //printf("DEBUG: %d\n", ch);
        if(ch == 224){
            ch = _getch();
            //printf("DEBUG: %d\n", ch);
        }
        
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

    clear_lines(0, buffer_info.dwSize.Y);
    setCursorVisibility(TRUE);

    return 0;
}