cat extended_stddef.h > all_in_one.c
echo                 >> all_in_one.c
cat extended_conio.h >> all_in_one.c
echo                 >> all_in_one.c
cat list.h           >> all_in_one.c
echo                 >> all_in_one.c
cat quotes.h         >> all_in_one.c
echo                 >> all_in_one.c
cat ui.h             >> all_in_one.c
echo                 >> all_in_one.c
cat main_activity.c  >> all_in_one.c
sed -i '/#include/d' all_in_one.c
sed -i '/^\s*\/\/TODO/d' all_in_one.c
sed -i 's/\/\/TODO.*//g' all_in_one.c
cat > all_in_one_final.c <<EOL
#include <stdio.h>
#include <conio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

EOL

cat all_in_one.c >> all_in_one_final.c