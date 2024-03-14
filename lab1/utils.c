#include "header/utils.h"

int string_compare(const char* str1, const char* str2) {
    // Compare characters until reaching the end of either string or a newline in str1
    while (*str1 && *str1 != '\n' && *str2) {
        if (*str1 != *str2) {
            // If characters differ, return the difference between them
            return (*(const unsigned char*)str1 - *(const unsigned char*)str2);
        }
        str1++;
        str2++;
    }
    
    // If we reached a newline in str1, consider it equivalent to the end of the string
    if (*str1 == '\n') {
        str1++;
    }
    
    // If both strings end at the same position, they are equal; otherwise, return the difference
    return (*(const unsigned char*)str1 - *(const unsigned char*)str2);
}
