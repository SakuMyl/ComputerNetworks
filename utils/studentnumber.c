#include <stdio.h>
#include <stdlib.h>

char *student_number()
{
    char *number = malloc(7);
    if (!number) return NULL;
    printf("Student number: ");
    int ret = scanf("%s", number);
    if (!ret) {
        free(number);
        return NULL;
    }
    return number;
}
