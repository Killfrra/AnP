// Agreement:
// Main is a bridge between UI, List and Quotes
// Defines should be undefined as soon as they not needed anymore
// Naming format: class_name_method
// And remember, not a word in Russian!

//#include <stdio.h>
#include <windows.h>
#include "extended_conio.h"
#include "extended_stddef.h"
#include "list.h"
#include "ui.h"

#define BOTTOM_LINE (buffer_info.dwSize.Y - 1)
#define RIGHT_CHAR  (buffer_info.dwSize.X - 1)

void print_to_status(char * str){
    setCursorPosition(0, BOTTOM_LINE);
    printf(str);
}

void print_error_or_mistake(char * str){
    clear_lines(1, 3);
    mistake_quote();
    print_to_status(str);
}

void menu_add(){
    if(link_layer == SEARCH)
        return;
    
    print_to_status("Enter to submit. Esc to exit");
    
    char ret = draw_editor(last_readed);
    if(ret == KEY_ENTER){
        for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
            if(cur->data.gradebook_number == last_readed->data.gradebook_number){
                print_error_or_mistake("KEY ALREADY EXIST");
                return;
            }
        list_add(last_readed);
        if(last_readed == HEAD)
            scroll_set_head(HEAD);
        last_readed = list_element_new();
        element_zerofill(last_readed);
        //TODO: clean_lines?
        start_quote();
        redraw_scroll();
        print_to_status("Element added");
    }
}

void menu_remove(){
    //if(link_layer == SEARCH)
    //    return;
    if(scroll_selected_element){
        ListElement * to_delete = scroll_selected_element;
        
        if(scroll_first_element_on_screen == HEAD){
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

        //TODO: remove from booth!
        if(link_layer == SEARCH){
            list_remove(to_delete);
            link_layer = SHOW;
            list_remove(to_delete);
            link_layer = SEARCH;
        } else
            list_remove(to_delete);

        remove_quote();
        redraw_scroll();
        print_to_status("Element removed");
    } else
        print_error_or_mistake("Nothing to remove");
}

void menu_edit(){
    if(scroll_selected_element){
        print_to_status("Editing element. Changes applied immidiately");
        //TODO: option to discard changes?
        draw_editor(scroll_selected_element);
        edit_quote();
        redraw_scroll();
        print_to_status("Element changed");
    } else
        print_error_or_mistake("Nothing to edit");
}

unsigned char sort_by_field = 0;
void menu_sort(){
    if(list_len > 1){

        if(header_select_column(&sort_by_field) == KEY_ESC)
            return;

        merge_sort(sort_by_field);
        //scroll_set_head(HEAD);
        ListElement * cur = scroll_selected_element;
        for(unsigned int i = 0; i < scroll_selected_element_pos; i++){
            if(!cur->PREV){
                scroll_selected_element_pos = i;
                break;
            } else
                cur = cur->PREV;
        }
        scroll_first_element_on_screen = cur;
        //TODO: clean_lines?
        start_quote();
        redraw_scroll();
        print_to_status("List sorted");
    } else
        print_error_or_mistake("Nothing to sort");
}

unsigned char search_by_field = 0;
void menu_search(){
    redraw_header(search_by_field);
    
    if(header_select_column(&search_by_field) == KEY_ESC)
        goto exit;

    Field field = list_element_fields[search_by_field];
    setCursorPosition(0, EDITOR_POSY);
    printf("Value: ");
    setCursorPosition(len("Value: ") + field.len, EDITOR_POSY);
    char mode = 'o';
    printf(" Mode: %c", mode);
    
    setCursorVisibility(TRUE);

    char ret = ARROW_RIGHT;
    char * field_value_ptr = (char * ) last_readed + field.offset; //TODO: rename
    Field mode_field = { prop: { values: "\3o+-" } };

    loop { //TODO: make shorter
        ret = field.read_func(ret, len("Value: ") - 1, field_value_ptr, field);
        if(ret == KEY_ENTER)
            break;
        else if(ret == KEY_ESC)
            goto exit;
        ret = read_char(ret, len("Value: ") + field.len + len(" Mode: ") - 1, &mode, mode_field);
        if(ret == KEY_ENTER)
            break;
        else if(ret == KEY_ESC)
            goto exit;
    }

    setCursorVisibility(FALSE);

    link_layer = SEARCH;
    char field_size = field.size;
    size_t field_offset = field.offset;

    //TODO: wrap into func
    if((char *) field.read_func == (char *) read_string){
        field_size = field_value_ptr[0];
        field_offset++;
        field_value_ptr++;
    }

    if(mode == 'o'){
        heads[SEARCH] = NULL;
        tails[SEARCH] = NULL;
        list_lengths[SEARCH] = 0;
    }
    if(mode != '-')
        for(ListElement * cur = heads[SHOW]; cur; cur = cur->link[2])
            if(memcmp((char *) cur + field_offset, field_value_ptr, field_size) == 0)
                list_add(cur);

    //TODO: '-' mode

    redraw_menu();
    scroll_set_head(heads[SEARCH]);
    redraw_scroll();
    print_to_status("Search completed");

exit:
    clear_lines(1, 3);
    start_quote();
}

void menu_close_search(){
    link_layer = SHOW;
    selected_menu_item = 0;
    redraw_menu();
    //TODO: clean_lines?
    start_quote();
    scroll_set_head(HEAD);
    redraw_scroll();
    print_to_status("Showing elements");
}

char read_filename(char * filename){
    setCursorPosition(0, EDITOR_POSY);
    printf("Filename: ");

    setCursorVisibility(TRUE);
    
    Field file_field = { len: FILEDATA_NAME_LEN, prop: { allow: ALLOW_DIGITS | ALLOW_SPECIAL } };
    int ret = ARROW_RIGHT;
    loop {
        ret = read_string(ret, len("Filename: ") - 1, filename, file_field);
        if(ret == KEY_ENTER || ret == KEY_ESC)
            goto exit;
    }

exit:
    setCursorVisibility(FALSE);
    return ret;
}

void menu_export(){
    //link_layer = SHOW;
    char filename[FILEDATA_NAME_LEN] = {0};
    char ret = read_filename(filename);
    if(ret == KEY_ESC)
        goto exit;

    FILE * file = fopen(&filename[1], "w");
    if(!file){
        print_error_or_mistake("Error creating file");
        return;
    }

    for(ListElement * cur = HEAD; cur; cur = cur->NEXT)
        if(element_print_to_txt(file, cur) <= 0){
            print_error_or_mistake("Error writing to file");
            fclose(file);
            return;
        }
    
    fflush(file);
    fclose(file);
    print_to_status("Successfully exported");

exit:
    clear_lines(1, 3);
    start_quote();
}

void menu_import(){
    //TODO: wrap into func?
    link_layer = SHOW;
    char filename[FILEDATA_NAME_LEN] = {0};
    char ret = read_filename(filename);
    if(ret == KEY_ESC)
        goto exit;

    FILE * file = fopen(&filename[1], "r");
    if(!file){
        print_error_or_mistake("Error opening file");
        return;
    }

    list_free();

    while(element_read_from_txt(file, last_readed) >= 0){
		list_add(last_readed);
		last_readed = list_element_new();
	}

    element_zerofill(last_readed);
    scroll_set_head(HEAD);
    redraw_scroll();
    print_to_status("Successfully imported");

exit: //TODO: func?
    clear_lines(1, 3);
    start_quote();
}

void menu_process(){
    list_process();
    redraw_menu();
    //TODO: clean_lines?
    start_quote();
    scroll_set_head(HEAD);
    redraw_scroll();
}

int main(){

    //SetConsoleCP(866);
    //SetConsoleOutputCP(866);

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleCursorInfo(stdout_handle, &cursor_info);
    adjust_buffer();
    
    list_autoload(); // also allocs last_readed
    scroll_set_head(HEAD);
    element_zerofill(last_readed);
    
    setCursorVisibility(FALSE);
    clear_lines(0, buffer_info.dwSize.Y);
    draw_menu();
    draw_header(0);
    scroll_last_line = BOTTOM_LINE - 1;
    draw_scroll();
    
    start_quote();

    loop {
    
        //clear_lines(BOTTOM_LINE, BOTTOM_LINE);
        //setCursorPosition(0, BOTTOM_LINE);
        //printf("Showing elements...");

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

    exit_quote();
    
    setCursorVisibility(TRUE);
    restore_buffer();
    
    list_autosave();
	list_release_memory();

    return 0;
}
