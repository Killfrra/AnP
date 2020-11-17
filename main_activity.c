// ??????????:
// ??????? ?????? - ??? list_element ??? ?????? element
// ??????? ???? - ??? menu_item ??? ?????? item
// ???????????????? ?????? - ??? scroll
// ?????????????? ???????? - editor
// ?????????? ? ?????? ????????? ??? "?????_????????"
// ???? ???? ???? ? ??????? (pos), ?? ?? ????????

//#include <stdio.h>
#include <windows.h>
#include "extended_conio.h"
#include "extended_stddef.h"
#include "list.h"
#include "ui.h"

ListElement * last_readed;

#define BOTTOM_LINE (buffer_info.dwSize.Y - 1)
#define RIGHT_CHAR  (buffer_info.dwSize.X - 1)

void menu_add(){
    clear_lines(BOTTOM_LINE, BOTTOM_LINE);
    setCursorPosition(0, BOTTOM_LINE);
    printf("Adding element. Enter to submit");
    
    char ret = draw_editor(last_readed);
    if(ret == KEY_ENTER){
        list_add(last_readed);
        if(last_readed == head)
            scroll_set_head(head);
        last_readed = new(ListElement);
        element_zerofill(last_readed);
    }
    redraw_scroll();
}

void menu_edit(){
    if(scroll_selected_element){
        clear_lines(BOTTOM_LINE, BOTTOM_LINE);
        setCursorPosition(0, BOTTOM_LINE);
        printf("Editing element. Changes are saved automatically");
        
        draw_editor(scroll_selected_element);
        //TODO: option to discard changes?
        redraw_scroll();
    } else {
        setCursorPosition(0, BOTTOM_LINE);
        printf("Nothing to edit");
    }
}

void menu_remove(){
    if(scroll_selected_element){
        ListElement * to_delete = scroll_selected_element;
        
        if(scroll_first_element_on_screen == head){
            // to_delete == scroll_first_element_on_screen
            if(scroll_selected_element_pos == 0)
                scroll_set_head(to_delete->NEXT);
            else {
                scroll_selected_element_pos--;
                scroll_selected_element = to_delete->PREV;
            }
        } else {
            scroll_first_element_on_screen = scroll_first_element_on_screen->PREV;
            scroll_selected_element = to_delete->PREV;
        }

        list_remove(to_delete);
        redraw_scroll();
    } else {
        setCursorPosition(0, BOTTOM_LINE);
        printf("Nothing to remove");
    }
}

void menu_sort(){
    if(list_len > 1){
        merge_sort();
        //scroll_set_head(head);
        ListElement * cur = scroll_selected_element;
        for(unsigned int i = 0; i < scroll_selected_element_pos; i++){
            if(!cur->PREV){
                scroll_selected_element_pos = i;
                break;
            } else
                cur = cur->PREV;
        }
        scroll_first_element_on_screen = cur;
        redraw_scroll();
    }
}

unsigned char search_by_field = 0;
void menu_search(){
    redraw_header(search_by_field);
    loop {
        int ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_RIGHT){
                search_by_field = (search_by_field + 1) % len(list_element_fields);
                redraw_header(search_by_field);
            } else if(ch == ARROW_LEFT){
                search_by_field = (search_by_field - 1) % len(list_element_fields);
                redraw_header(search_by_field);
            }
        } else if(ch == KEY_ENTER)
            break;
    }

    Field list_field = list_element_fields[search_by_field];
    setCursorPosition(0, EDITOR_POSY);
    printf("Value: ");
    setCursorPosition(len("Value: ") + list_field.size, EDITOR_POSY);
    char mode = 'o';
    printf(" Mode: %c", mode);
    
    setCursorVisibility(TRUE);

    char ret = ARROW_RIGHT;
    char * elem_offset = (char * ) last_readed + list_field.offset;
    Field mode_field = { read_char, "mode", 0, 1, { values: "\3o+-" } };

    loop {        
        ret = list_field.read_func(ret, len("Value: ") - 1, elem_offset, list_field);
        if(ret == KEY_ENTER || ret == KEY_ESC)
            break;
        ret = mode_field.read_func(ret, len("Value: ") + list_field.size + len(" Mode: ") - 1, &mode, mode_field);
        if(ret == KEY_ENTER || ret == KEY_ESC)
            break;
    }

    setCursorVisibility(FALSE);
}

int main(){

    //SetConsoleCP(866);
    //SetConsoleOutputCP(866);

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleCursorInfo(stdout_handle, &cursor_info);
    adjust_buffer();
    
    last_readed = new(ListElement);
    element_zerofill(last_readed);
    
    setCursorVisibility(FALSE);
    clear_lines(0, buffer_info.dwSize.Y);
    draw_menu();
    draw_header(0);
    scroll_last_line = BOTTOM_LINE - 1;
    draw_scroll();
    
    loop {
        //setCursorPosition(0, 1);
        //puts(" ? ?????, ? ?????? ? ???? ");

        clear_lines(BOTTOM_LINE, BOTTOM_LINE);
        setCursorPosition(0, BOTTOM_LINE);
        printf("Showing elements...");

        int ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_UP)
                scroll_scroll(UP);
            else if(ch == ARROW_DOWN)
                scroll_scroll(DOWN);
            else if(ch == ARROW_LEFT)
                scroll_menu(LEFT);
            else if(ch == ARROW_RIGHT)
                scroll_menu(RIGHT);
        } else if(ch == KEY_ENTER)
            menu_items[selected_menu_item].func();
        else if(ch == KEY_ESC)
            break;
    }

    clear_lines(0, buffer_info.dwSize.Y);
    setCursorPosition(0, 0);
    setCursorVisibility(TRUE);
    restore_buffer();
    
    free(last_readed);

    return 0;
}
