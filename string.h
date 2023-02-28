#define MAX_LENGTH 32

typedef struct string
{
    int length;
    char *string;
} string;

void string_init(string *s);
void string_append(string *s, char c);
void string_clear(string *s);
char *string_get_string(string s);
int string_length(char *s);