#include <stdio.h>
#include <windows.h>

#define loop while(1)
#define new(T) malloc(sizeof(T))
#define len(x)  (sizeof(x) / sizeof((x)[0]))

#define ARROW_UP 72
#define ARROW_DOWN 80
#define ARROW_LEFT 75
#define ARROW_RIGHT 77

#define BACKGROUND_WHITE (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)

#define TOP_OFFSET 1
#define BUFFER_Y 3

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
void clear(short from, short to){
    COORD coord = { 0, from };
    DWORD written;
    FillConsoleOutputCharacter(stdout_handle, ' ', buffer_info.dwSize.X * to, coord, &written);
    FillConsoleOutputAttribute(stdout_handle, buffer_info.wAttributes, buffer_info.dwSize.X * to, coord, &written);
    setCursorPosition(0, from);
}

void fill(short from_x, short from_y, DWORD _len, WORD attr){
    DWORD written;
    COORD coord = { from_x, from_y };
    FillConsoleOutputAttribute(stdout_handle, attr, _len, coord, &written);
}

void redraw_list(){

    setCursorPosition(0, 0); //clear(0, BUFFER_Y);

    if(head){
        ListElement * cur = first_element_on_screen;
        for(int i = 0; cur && i < BUFFER_Y; i++){
            print(cur);
            cur = cur->NEXT;
        }
        fill(0, selected_element_pos_on_screen, buffer_info.dwSize.X, BACKGROUND_WHITE);
    } else
        puts("Nothing to show");
}

typedef enum { UP, DOWN } Vertical;
void scroll_list(Vertical dir){

    if(selected_element->DIR(dir)){

        if(dir == UP && selected_element_pos_on_screen == 0){
            first_element_on_screen = first_element_on_screen->PREV;
            selected_element = first_element_on_screen;
            redraw_list();
        } else if(dir == DOWN && selected_element_pos_on_screen == BUFFER_Y - 1){
            first_element_on_screen = first_element_on_screen->NEXT;
            selected_element = selected_element->NEXT;
            redraw_list();
        } else {

            fill(0, selected_element_pos_on_screen, buffer_info.dwSize.X - 1, buffer_info.wAttributes);

            // char offset[2] = { -1, 1 };
            selected_element_pos_on_screen += dir * 2 - 1; //offset[dir];
            selected_element = selected_element->DIR(dir);

            fill(0, selected_element_pos_on_screen, buffer_info.dwSize.X - 1, BACKGROUND_WHITE);
        }
    } //else
      //  Beep(750, 100);
}

typedef struct {
    char * name;
    char   name_len;
    void (* func)();
} MenuItem;

void empty(){}

#define item(name, func) { name, len(name) - 1, func }
MenuItem menu_items[] = {
    item( "+",  empty ),
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
    //FillConsoleOutputCharacter(stdout_handle, ' ', buffer_info.dwSize.X - menu_len, coord, &written);
    SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
    //printf("\nDEBUG: %d %d %lu", buffer_info.dwSize.X, menu_len, written);
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
        //printf("DEBUG: %s %d\n", menu_items[i].name, (int) menu_items[i].name_len);
    }
    //return 0;
    
    clear(0, buffer_info.dwSize.Y);

    add("one"); // one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one");
    add("two"); // two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two");
    add("three"); // three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three");
    add("four"); // four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four");
    add("five"); // five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five");
    
    setCursorVisibility(FALSE);
    redraw_menu();
    redraw_list();
    
    loop {
        int ch = _getch();
        if(ch == 224)
            ch = _getch();
        
        if(ch == ARROW_UP)
            scroll_list(UP);
        else if(ch == ARROW_DOWN)
            scroll_list(DOWN);
        else if(ch == ARROW_LEFT)
            scroll_menu(LEFT);
        else if(ch == ARROW_RIGHT)
            scroll_menu(RIGHT);
        else
            break;
    }

    setCursorVisibility(TRUE);
    clear(0, BUFFER_Y);

    return 0;
}