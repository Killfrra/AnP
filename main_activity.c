#include <stdio.h>
#include <windows.h>

#define new(T) malloc(sizeof(T))
#define loop while(1)

#define ARROW_UP 72
#define ARROW_DOWN 80
#define ARROW_LEFT 75
#define ARROW_RIGHT 77

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
ListElement * firstElementOnScreen = NULL;
ListElement * currentSelected = NULL;
int selectedPosOnScreen = -1;

void add(char * name){
    ListElement * el = new(ListElement);
    el->data.name = name;
    if(!head){
        el->PREV = NULL;
        head = el;

        firstElementOnScreen = head;
        currentSelected = head;
        selectedPosOnScreen = 0;

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
    setCursorPosition(0, from);
}

void redraw_list(){

    setCursorPosition(0, 0); //clear(0, BUFFER_Y);

    if(head){
        ListElement * cur = firstElementOnScreen;
        for(int i = 0; cur && i < BUFFER_Y; i++){
            if(selectedPosOnScreen == i)
                putchar('>');
            else
                putchar(' ');
            print(cur);
            cur = cur->NEXT;
        }
    } else
        puts("Nothing to show");
}

typedef enum { UP, DOWN } Horizontal;
void scroll_list(Horizontal dir){

    if(currentSelected->DIR(dir)){

        if(dir == UP && selectedPosOnScreen == 0){
            firstElementOnScreen = firstElementOnScreen->PREV;
            currentSelected = firstElementOnScreen;
            redraw_list();
        } else if(dir == DOWN && selectedPosOnScreen == BUFFER_Y - 1){
            firstElementOnScreen = firstElementOnScreen->NEXT;
            currentSelected = currentSelected->NEXT;
            redraw_list();
        } else {

            setCursorPosition(0, selectedPosOnScreen);
            putchar(' ');
            print(currentSelected);

            // char offset[2] = { -1, 1 };
            selectedPosOnScreen += dir * 2 - 1; //offset[dir];
            currentSelected = currentSelected->DIR(dir);

            setCursorPosition(0, selectedPosOnScreen);
            putchar('>');
            print(currentSelected);
        }
    } //else
      //  Beep(750, 100);
}

int main(){

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(stdout_handle, &cursor_info);
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);

    clear(0, buffer_info.dwSize.Y);

    add("one"); // one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one one");
    add("two"); // two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two two");
    add("three"); // three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three three");
    add("four"); // four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four four");
    add("five"); // five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five five");
    
    setCursorVisibility(FALSE);
    redraw_list();
    
    loop {
        int ch = _getch();
        if(ch == 224)
            ch = _getch();
        
        if(ch == ARROW_UP)
            scroll_list(UP);
        else if(ch == ARROW_DOWN)
            scroll_list(DOWN);
        else
            break;
    }

    setCursorVisibility(TRUE);
    clear(0, BUFFER_Y);

    return 0;
}