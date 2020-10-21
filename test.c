#define UNICODE
#define _UNICODE
#define _GLIBCXX_USE_WCHAR_T

#include <conio.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <locale.h>

int main()
{
    setlocale(LC_ALL, "");
    //SetConsoleCP(65001);
    //SetConsoleOutputCP(65001);
    //_setmode(_fileno(stdin),  _O_U8TEXT);
    //_setmode(_fileno(stdout), _O_U8TEXT);
    printf(u8"Тестовая строка %I64u\n", sizeof(char));
    wint_t ch = _getch();
    printf("Введён: %c %d\n", ch, (int) ch);
    return 0;
}