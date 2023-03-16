#include "string.h"
#include <stddef.h>

int strcmp(char *s1 ,char *s2){
    int i;

    for(i =0 ;i<strlen(s1);i++){
        if(s1[i]!=s2[i])
            return s1[i]-s2[i];
    }

    return s1[i]-s2[i];

}

int strlen(char *s){
    int i=0;

    while(1){
        if(*(s+i) == '\0')
            break;
        else   
            i++;
    }

    return i;
}

void str_changend(char *str){
    while(*str != '\0'){
        if(*str == '\n'){
            *str = '\0';
            return;
        }
        str++;
    }
}