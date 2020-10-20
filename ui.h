#ifndef CUSTOM_UI
#define CUSTOM_UI

#include "extended_conio.h"
#include "extended_stddef.h"

ListElement * scroll_first_element_on_screen = NULL;
ListElement * scroll_selected_element = NULL;
        int   scroll_selected_element_pos = -1;

short scroll_first_line = 1;
short scroll_last_line = 5;

void scroll_set_head(ListElement * el){
    scroll_first_element_on_screen = el;
    scroll_selected_element = el;
    scroll_selected_element_pos = 0;
}

#define draw_scroll redraw_scroll
void redraw_scroll(){

    setCursorPosition(0, scroll_first_line); //clear_lines(0, BUFFER_Y);

    if(head){
        ListElement * cur = scroll_first_element_on_screen;
        for(int i = 0; cur && i < (scroll_last_line - scroll_first_line); i++){
            if(i == scroll_selected_element_pos){
                SetConsoleTextAttribute(stdout_handle, BACKGROUND_WHITE);
                element_print(cur);
                SetConsoleTextAttribute(stdout_handle, buffer_info.wAttributes);
            } else
                element_print(cur);
            cur = cur->NEXT;
        }
        setColor(0, scroll_selected_element_pos + scroll_first_line, buffer_info.dwSize.X - 1, BACKGROUND_WHITE);
        
        setCursorPosition(buffer_info.dwSize.X - 1, scroll_first_line);
        if(scroll_first_element_on_screen != head)
            putchar('^');
        else
            putchar(' ');

        setCursorPosition(buffer_info.dwSize.X - 1, scroll_last_line);
        if(cur)
            putchar('v');
        else
            putchar(' ');
    
    } else {
        clear_lines(scroll_first_line, scroll_last_line);
        puts("\n Nothing to show");
    }
}

typedef enum { UP, DOWN } Vertical;
void scroll_scroll(Vertical dir){
    if(!head) return;
    if(scroll_selected_element->DIR(dir)){

        if(dir == UP && scroll_selected_element_pos == 0){
            scroll_first_element_on_screen = scroll_first_element_on_screen->PREV;
            scroll_selected_element = scroll_first_element_on_screen;
            redraw_scroll();
        } else if(dir == DOWN && scroll_selected_element_pos == scroll_last_line - scroll_first_line - 1){
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
    char posx = 0;

    clear_lines(1, POSY);
    setCursorPosition(0, POSY);
    element_print(elem);
    repeat(0, 3, buffer_info.dwSize.X, '_');
    setCursorVisibility(TRUE);

    loop {
        Field field = list_element_fields[current_field];
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
                posx -= list_element_fields[current_field].size + 1;
            }
        } else if(ret == KEY_ESC || ret == KEY_ENTER)
            break;
        else if(current_field == len(list_element_fields) - 1){
            if(ret != KEY_ENTER){
                current_field = 0;
                posx = 0;
            } else
                break;
        } else {
            posx += list_element_fields[current_field].size + 1;
            current_field++;
        }
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

#define item(name, func) { name, len(name) - 1, func }
MenuItem menu_items[] = {
    item("+",  menu_add ),
    item("-",  menu_remove),
    item("Edit",  menu_edit),
    item("Search", empty_func),
    item("Sort", empty_func),
    item("Export", empty_func),
    item("Import", empty_func),
    item("Save", empty_func),
    item("Process", empty_func)
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

#endif