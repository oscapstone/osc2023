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

int startwith(char* str1, char* str2){
  while(1){
    if(*str2 == '\0'){
      return 0;
    }
    if(*str1 != *str2){
      return 1;
    }
    
    str1 ++;
    str2 ++;
  }
}

int find_in_str(char* str1, char target){
  int pos = 0;
  while(1){
    if(*(str1 + pos) == target) return pos;
    if(*(str1 + pos) == '\0') return -1;
    pos ++;
  }
}
