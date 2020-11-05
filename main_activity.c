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
    merge_sort();
    scroll_set_head(head);
    redraw_scroll();
}

int main(){

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(stdout_handle, &cursor_info);
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
    
    last_readed = new(ListElement);
    element_zerofill(last_readed);

    /*
    add("one"); // one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one");
    add("two"); // two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two");
    add("three"); // three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three");
    add("four"); // four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four");
    add("five"); // five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five");
    */
    
    setCursorVisibility(FALSE);
    clear_lines(0, buffer_info.dwSize.Y);
    draw_menu();
    scroll_last_line = BOTTOM_LINE - 1;
    draw_scroll();

    /*
    setCursorPosition(0, 1);
    print_element(last_readed);
    
    draw_editor(last_readed);
    
    setCursorPosition(0, 1);
    print_element(last_readed);
    */
    
    loop {
        setCursorPosition(0, 1);
        puts(" ?????? ??????? ??? ????? ???????? ??????????? ??? ");

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
    setCursorVisibility(TRUE);

    free(last_readed);

    return 0;
}