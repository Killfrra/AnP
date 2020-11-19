#ifndef CUSTOM_UI
#define CUSTOM_UI

#define MENU_POSY	0
#define EDITOR_POSY	2 // �1
#define HEADER_POSY 4
#define SCROLL_POSY 5
#define E404_POSY	6

#include "extended_conio.h"
#include "extended_stddef.h"
#include "quotes.h"

ListElement * scroll_first_element_on_screen = NULL;
ListElement * scroll_selected_element = NULL;
unsigned int  scroll_selected_element_pos = 0;

unsigned short scroll_first_line = SCROLL_POSY;
unsigned short scroll_last_line = 5;

void scroll_set_head(ListElement * el){
    scroll_first_element_on_screen = el;
    scroll_selected_element = el;
    scroll_selected_element_pos = 0;
}

#define draw_scroll redraw_scroll
void redraw_scroll(){

    setCursorPosition(0, scroll_first_line); //clear_lines(0, BUFFER_Y);

    if(HEAD){
        ListElement * cur = scroll_first_element_on_screen;
        unsigned int drawn = 0;
        for(; cur && drawn < (scroll_last_line - scroll_first_line + 1); drawn++){
            if(drawn == scroll_selected_element_pos){
                SetConsoleTextAttribute(stdout_handle, BACKGROUND_WHITE);
                element_print(cur);
                SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
            } else
                element_print(cur);
            cur = cur->NEXT;
        }
        setColor(0, scroll_selected_element_pos + scroll_first_line, buffer_info.dwSize.X - 1, BACKGROUND_WHITE);
        
        setCursorPosition(buffer_info.dwSize.X - 1, scroll_first_line);
        if(scroll_first_element_on_screen != HEAD)
            putchar('^');
        else
            putchar(' ');

        setCursorPosition(buffer_info.dwSize.X - 1, scroll_last_line);
        if(cur)
            putchar('v');
        else
            putchar(' ');

        clear_lines(scroll_first_line + drawn, scroll_last_line);
    
    } else {
        clear_lines(scroll_first_line, scroll_last_line);
        setCursorPosition(0, E404_POSY);
		puts(" Nothing to show");
    }
}

typedef enum { UP, DOWN } Vertical;
void scroll_scroll(Vertical dir){
    if(!HEAD) return;
    if(scroll_selected_element->DIR(dir)){

        if(dir == UP && scroll_selected_element_pos == 0){
            scroll_first_element_on_screen = scroll_first_element_on_screen->PREV;
            scroll_selected_element = scroll_first_element_on_screen;
            redraw_scroll();
        } else if(dir == DOWN && scroll_selected_element_pos == (scroll_last_line - scroll_first_line)){
            scroll_first_element_on_screen = scroll_first_element_on_screen->NEXT;
            scroll_selected_element = scroll_selected_element->NEXT;
            redraw_scroll();
        } else {

            setColor(0, scroll_selected_element_pos + scroll_first_line, buffer_info.dwSize.X - 1, buffer_info.wAttributes);

            // char offset[2] = { -1, 1 };
            scroll_selected_element_pos += dir * 2 - 1; //offset[dir];
            scroll_selected_element = scroll_selected_element->DIR(dir);

            setColor(0, scroll_selected_element_pos + scroll_first_line, buffer_info.dwSize.X - 1, BACKGROUND_WHITE);
        }
    } //else
      //  Beep(750, 100);
}

char draw_editor(ListElement * elem){
    unsigned char current_field = 0;
    char ret = ARROW_RIGHT;
    
    clear_lines(EDITOR_POSY - 1, EDITOR_POSY + 1);
    setCursorPosition(0, EDITOR_POSY);
    element_print(elem);
    //repeat(0, 3, buffer_info.dwSize.X, '_');
    setCursorVisibility(TRUE);

    loop {
        Field field = list_element_fields[current_field];
        char * offset = (char * ) elem + field.offset;
        ret = field.read_func(ret, field.posx, offset, field);

        if(ret == ARROW_LEFT){
            if(current_field == 0)
                current_field = len(list_element_fields) - 1;
            else
                current_field--;
        } else if(ret == KEY_ESC || ret == KEY_ENTER) //TODO: only esc
            break;
        else if(current_field == len(list_element_fields) - 1){
            if(ret == KEY_ENTER)
                break;
            else
                current_field = 0;
        } else
            current_field++;
    }

    setCursorVisibility(FALSE);
    clear_lines(1, 3);

    return ret;
}

typedef struct {
    char * name;
    char   name_len;
    void (* func)();
} MenuItem;

void empty_func(){}

void menu_add();
void menu_remove();
void menu_edit();
void menu_sort();
void menu_search();
void menu_close_search();

#define item(name, func) { name, len(name) - 1, func }
MenuItem menu_items[] = {
    item("+",  menu_add ),
    item("-",  menu_remove),
    item("Edit",  menu_edit),
    item("Search", menu_search),
    item("Sort", menu_sort),
    item("Export", empty_func),
    item("Import", empty_func),
    item("Save", empty_func),
    item("Process", empty_func),
    item(" X ", menu_close_search)
};

signed char selected_menu_item = 0;
int menu_len = 0;
void redraw_menu(){
    setCursorPosition( 0, 0);
    SetConsoleTextAttribute(stdout_handle, BACKGROUND_BLUE);
    unsigned char menu_len = len(menu_items) - (link_layer == SHOW);
    for(unsigned char i = 0; i < menu_len; i++){
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

    unsigned char menu_len = len(menu_items) - (link_layer == SHOW);
    selected_menu_item += dir * 2 - 1;
    if(selected_menu_item < 0)
        selected_menu_item = menu_len - 1;
    else if(selected_menu_item == menu_len)
        selected_menu_item = 0;
    
    redraw_menu();
}

void redraw_header(unsigned char selected_header_item){
    setCursorPosition(0, HEADER_POSY);
    SetConsoleTextAttribute(stdout_handle, BACKGROUND_GRAY);
    for(unsigned char i = 0; i < len(list_element_fields); i++){
        if(i == selected_header_item){
            SetConsoleTextAttribute(stdout_handle, BACKGROUND_WHITE);
            printf(" %s ", list_element_fields[i].name);
            SetConsoleTextAttribute(stdout_handle, BACKGROUND_GRAY);
        } else
            printf(" %s ", list_element_fields[i].name);
    }
    SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
}

void draw_header(unsigned char selected_header_item){
    setColor(0, HEADER_POSY, buffer_info.dwSize.X, BACKGROUND_GRAY);
    redraw_header(selected_header_item);
}

char header_select_column(unsigned char * in_out){
    unsigned char selected_column = *in_out;
    int ch;
    loop {
        ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_RIGHT){
                selected_column = (selected_column + 1) % len(list_element_fields);
                redraw_header(selected_column);
            } else if(ch == ARROW_LEFT){
                selected_column = (selected_column - 1) % len(list_element_fields);
                redraw_header(selected_column);
            }
        } else if(ch == KEY_ENTER || ch == KEY_ESC)
            break;
    }
    *in_out = selected_column;
    return ch;
}

#endif
