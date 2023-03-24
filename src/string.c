int strcmp(char* str1, char* str2){
  while(1){
    if(*str1 != *str2){
      return 1;
    }
    if(*str1 == '\0'){
      return 0;
    }

    str1 ++;
    str2 ++;
  }
}
