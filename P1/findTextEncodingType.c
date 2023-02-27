#include <stdio.h>
#include <locale.h>
#include <langinfo.h>
 
// main function
int main() {
    setlocale(LC_ALL, "");
    char* locstr = setlocale(LC_CTYPE, NULL);
    char* encoding = nl_langinfo(CODESET);
    printf("Locale is %s\n", locstr);
    printf("Encoding is %s\n", encoding);
    return 0;
}

/* 
    Locale is pt_PT.UTF-8
    Encoding is UTF-8
 */