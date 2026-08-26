#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_TOKENS 10
void parse_input(const char *input) {
    char *tokens[MAX_TOKENS];
    char *copy = strdup(input);
    int count = 0;
char *tok = strtok(copy, " \t\n");
while (tok && count < MAX_TOKENS) {
    tokens[count++] = tok;
    tok = strtok(NULL, " \t\n");
}

printf("Tokens:\n");
for (int i = 0; i < count; i++) {
    printf("[%d] %s\n", i+1, tokens[i]);
}

free(copy);
}
int main() {
    char buffer[100];
    while (1) {
        printf("Enter command (or 'exit'): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(buffer, "exit") == 0) break;
        parse_input(buffer);
    }
    return 0;
}
