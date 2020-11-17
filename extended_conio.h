#ifndef CONIO_EXTENSIONS
#define CONIO_EXTENSIONS

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "extended_stddef.h"

#define ARROW_UP 72
#define ARROW_DOWN 80
#define ARROW_LEFT 75
#define ARROW_RIGHT 77
#define KEY_ENTER 13
#define KEY_ESC 27
#define KEY_BACKSPACE 8
#define KEY_TAB 9

#define BACKGROUND_WHITE (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)
#define BACKGROUND_GRAY BACKGROUND_INTENSITY

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
    //setCursorPosition(0, from);
}

CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
void adjust_buffer(){
    //TODO: ??????, ??? ? ? ????? ??????
    //SMALL_RECT rect = { 32, 32, 128, 32 };
    //SetConsoleWindowInfo(stdout_handle, TRUE, &rect);
    GetConsoleScreenBufferInfo(stdout_handle, &orig_buffer_info);
    buffer_info = orig_buffer_info;
    buffer_info.dwSize.X = buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1;
    buffer_info.dwSize.Y = buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1;
    SetConsoleScreenBufferSize(stdout_handle, buffer_info.dwSize);
}

void restore_buffer(){
    SetConsoleScreenBufferSize(stdout_handle, orig_buffer_info.dwSize);
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

typedef struct field_struct {
    char (* read_func)(char enter_dir, short posx, void * dest, struct field_struct field);
    char * name;
    size_t offset;
    char size;
    union {
        char values[5];
        char allow_digits;
    } prop;
} Field;

#define EDITOR_POSY	2 //TODO: remove and get it from ui.h

char read_string(char enter_dir, short posx, char * dest, Field field){
    
    char buffer_size = field.size;
    char allow_digits = field.prop.allow_digits;

    char * buffer = &dest[1];
    unsigned char last_char = dest[0];
    unsigned char cursor_pos = 0;
    if(enter_dir == ARROW_LEFT)
        cursor_pos = last_char;

    setCursorPosition(posx, EDITOR_POSY);
    printf("%s", buffer);
    setCursorPosition(posx + cursor_pos, EDITOR_POSY);

    int ch;
    loop {
        
        ch = _getch();
        //printf("DEBUG: %d\n", ch);
        if(ch == 224){
            ch = _getch();
            //printf("DEBUG: %d\n", ch);
            if(ch == ARROW_LEFT){
                if(cursor_pos != 0){
                    setCursorPosition(posx + cursor_pos - 1, EDITOR_POSY);
                    cursor_pos--;
                } else
                    goto exit;
            } else if(ch == ARROW_RIGHT){
                if(cursor_pos != last_char){
                    setCursorPosition(posx + cursor_pos + 1, EDITOR_POSY);
                    cursor_pos++;
                } else
                    goto exit;
            } else if(ch == ARROW_UP){
                cursor_pos = 0;
                setCursorPosition(posx, EDITOR_POSY);
            } else if(ch == ARROW_DOWN){
                cursor_pos = last_char;
                setCursorPosition(posx + cursor_pos, EDITOR_POSY);
            }
        } else {
            //printf("DEBUG: %c not 244\n", ch);
            if(ch == KEY_ENTER || ch == KEY_ESC || ch == KEY_TAB)
                goto exit;
            else if(ch == KEY_BACKSPACE){
                if(cursor_pos != 0){
                    setCursorPosition(posx + cursor_pos - 1, EDITOR_POSY);
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
                    setCursorPosition(posx + cursor_pos, EDITOR_POSY);
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
                    setCursorPosition(posx + (++cursor_pos), EDITOR_POSY);
                    continue;
                }

                //TODO: think: оставлять ли последний char в buffer под '\0'?
                if(last_char == buffer_size){
                    setCursorPosition(posx, EDITOR_POSY + 1);
                    puts("Too many letters!");
                    setCursorPosition(posx + cursor_pos, EDITOR_POSY);
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

                setCursorPosition(posx + cursor_pos, EDITOR_POSY);
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

char read_fixed_int(char enter_dir, short posx, unsigned int * dest, Field field){

    unsigned char n_digits = field.size;
    
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

    setCursorPosition(posx, EDITOR_POSY);
    printf("%s", buffer);
    setCursorPosition(posx + cursor_pos, EDITOR_POSY);

    int ch;
    loop {
        ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_LEFT){
                if(cursor_pos != 0){
                    cursor_pos--;
                    setCursorPosition(posx + cursor_pos, EDITOR_POSY);
                } else
                    goto exit;
            } else if(ch == ARROW_RIGHT){
                if(cursor_pos != n_digits - 1){
                    cursor_pos++;
                    setCursorPosition(posx + cursor_pos, EDITOR_POSY);
                } else
                    goto exit;
            }
        } else {
            if(ch == KEY_ENTER || ch == KEY_ESC || ch == KEY_TAB)
                goto exit;
            else if(ch >= '0' && ch <= '9'){
                buffer[cursor_pos] = ch;
                putchar(ch);
                cursor_pos++;

                if(cursor_pos == n_digits)
                    goto exit;

                setCursorPosition(posx + cursor_pos, EDITOR_POSY);
            }
        }
    }

exit:
    *dest = atoi(buffer);
    return ch;
}

char read_fixed_short(char enter_dir, short posx, unsigned short * dest, Field field){
    unsigned int temp = *dest;
    char ret = read_fixed_int(enter_dir, posx, &temp, field);
    *dest = temp & 0xFFFF;
    return ret;
}

char read_fixed_date(char enter_dir, short posx, Date * dest, Field field){
    
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

    setCursorPosition(posx, EDITOR_POSY);
    puts(buffer);
    setCursorPosition(posx + cursor_pos, EDITOR_POSY);

    #define inc_cursor_pos() { \
        cursor_pos += 1 + (cursor_pos == 1 || cursor_pos == 4); \
        setCursorPosition(posx + cursor_pos, EDITOR_POSY); \
    }
    #define dec_cursor_pos() { \
        cursor_pos -= 1 + (cursor_pos == 3 || cursor_pos == 6); \
        setCursorPosition(posx + cursor_pos, EDITOR_POSY); \
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
            if(ch == KEY_ENTER || ch == KEY_ESC || ch == KEY_TAB)
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

char read_char(char enter_dir, short posx, char * dest, Field field){

    char * values = field.prop.values;

    char n_values = (values++)[0];
    char buffer = *dest;
    
    setCursorPosition(posx, EDITOR_POSY);
    putchar(buffer);
    setCursorPosition(posx, EDITOR_POSY);

    int ch;
    loop {
        ch = _getch();
        if(ch == 224){
            ch = _getch();
            if(ch == ARROW_LEFT || ch == ARROW_RIGHT)
                goto exit;
        } else {
            if(ch == KEY_ENTER || ch == KEY_ESC || ch == KEY_TAB)
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

#endif
