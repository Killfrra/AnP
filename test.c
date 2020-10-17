#include <stdio.h>
#include <string.h>

#define bool char
#define true 1
#define false 0
#define loop while(true)
#define len(x)  (sizeof(x) / sizeof((x)[0]))
#define foreach(i, array) for(size_t i = 0; i < len(array); i++)
#define isspace(c) (c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')

bool ignore(){
	if(getchar() != '\n'){
		puts("Оставшаяся часть будет проигнорирована");
		//mistake_quote();
		while(getchar() != '\n');
	}
	return true;
}

// name
// surname
// secondname

bool trim(char * buf, int size){
	int l, r;
	for(l = 0; l < size; l++)
		if(!isspace(buf[l]))
			break;
	if(l == size)
        return false;
    for(r = size - 1; r > l; r--)
        if(!isspace(buf[r]))
            break;
    memcpy(buf, buf + l, r - l + 1);
    buf[r - l + 1] = '\0';
    return true;
}

void gets(char * buf, int size, int len){ //TODO: optimize
	loop {
		fgets(buf, size, stdin);
        len = strlen(buf);
		if(!trim(buf, len))
            continue;
        return;
	}
}

int main(int argc, char const *argv[])
{
    char str[8];
    gets(str, 7, 6); // 6 + '\0'
    trim(str, 7);
    puts(str);
    return 0;
}
