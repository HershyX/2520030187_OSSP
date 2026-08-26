#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_HISTORY 10
#define MAX_CMD_LEN 100
char *history[MAX_HISTORY];
int history_count = 0;
void add_to_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    } else {
        free(history[0]); // release oldest
        for (int i = 1; i < MAX_HISTORY; i++) history[i-1] = history[i];
        history[MAX_HISTORY-1] = strdup(cmd);
    }
}
void show_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i+1, history[i]);
    }
}
int main() {
    char buffer[MAX_CMD_LEN];
    while (1) {
        printf("Enter command (or 'exit'): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0; // remove newline
        if (strcmp(buffer, "exit") == 0) break;
        add_to_history(buffer);
        show_history();
    }
    for (int i = 0; i < history_count; i++) free(history[i]);
    return 0;
}
