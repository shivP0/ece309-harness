/*
 * harness.c
 *
 * A minimal LLM-agent-style harness written in standard C.
 *
 * Features:
 * - Reads user input with fgets()
 * - Exits safely when the user types "exit"
 * - Routes "calc ..." commands to a calculator tool
 * - Routes other messages to a mock LLM function
 * - Keeps only the last 5 user/model conversation turns
 * - Uses dynamic memory safely and frees it before exiting
 *
 * Compile:
 *   gcc -Wall -Wextra -pedantic -std=c11 harness.c -o harness
 *
 * Run:
 *   ./harness
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Maximum number of user/assistant turns stored in history. */
#define MAX_HISTORY 5

/* Maximum input line length read by fgets(). */
#define INPUT_SIZE 512

/*
 * A Turn represents one complete conversation exchange:
 * - user: the text entered by the user
 * - response: the model or tool response
 *
 * These pointers are dynamically allocated using malloc().
 * That means we must later free them.
 */
typedef struct {
    char *user;
    char *response;
} Turn;

/*
 * ConversationHistory stores up to MAX_HISTORY turns.
 *
 * count tells us how many valid turns currently exist.
 * For example:
 * - count == 0 means history is empty
 * - count == 5 means history is full
 */
typedef struct {
    Turn turns[MAX_HISTORY];
    int count;
} ConversationHistory;

/*
 * safe_strdup()
 *
 * strdup() is common on POSIX systems, but it is not part of ISO C.
 * To keep this program closer to standard C, we implement a small
 * equivalent function ourselves.
 *
 * It allocates memory for a copy of text and returns the new pointer.
 * The caller becomes responsible for calling free() on the result.
 */
char *safe_strdup(const char *text)
{
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text);

    /*
     * Allocate one extra byte for the terminating '\0' character.
     */
    copy = malloc(length + 1);

    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, text);
    return copy;
}

/*
 * remove_newline()
 *
 * fgets() usually stores the newline character when the user presses Enter.
 * This function replaces that newline with '\0', making the input easier
 * to compare and process.
 */
void remove_newline(char *text)
{
    if (text == NULL) {
        return;
    }

    text[strcspn(text, "\n")] = '\0';
}

/*
 * contains_hello()
 *
 * Returns 1 if the input contains "hello", ignoring letter case.
 * Returns 0 otherwise.
 *
 * Example matches:
 * - "hello"
 * - "Hello there"
 * - "say HELLO please"
 */
int contains_hello(const char *input)
{
    const char *current;

    if (input == NULL) {
        return 0;
    }

    for (current = input; *current != '\0'; current++) {
        if (tolower((unsigned char)current[0]) == 'h' &&
            tolower((unsigned char)current[1]) == 'e' &&
            tolower((unsigned char)current[2]) == 'l' &&
            tolower((unsigned char)current[3]) == 'l' &&
            tolower((unsigned char)current[4]) == 'o') {
            return 1;
        }
    }

    return 0;
}

/*
 * mock_model()
 *
 * This function simulates a very small language model.
 *
 * It returns dynamically allocated memory. The caller must free()
 * the returned response when it is no longer needed.
 */
char *mock_model(const char *user_input)
{
    const char *greeting =
        "Hello! I am a mock LLM running inside a simple C agent harness.";

    char response_buffer[INPUT_SIZE + 100];

    if (user_input == NULL) {
        return safe_strdup("Mock model error: received NULL input.");
    }

    /*
     * Requirement: if input contains "hello", return a hardcoded greeting.
     */
    if (contains_hello(user_input)) {
        return safe_strdup(greeting);
    }

    /*
     * For other messages, return a simple echo-style response.
     * snprintf() is safer than sprintf() because it limits output size.
     */
    snprintf(response_buffer,
             sizeof(response_buffer),
             "Mock model response: You said: \"%s\"",
             user_input);

    return safe_strdup(response_buffer);
}

/*
 * calculator_tool()
 *
 * Parses one simple math expression:
 *
 *   number operator number
 *
 * Supported operators:
 *   +  addition
 *   -  subtraction
 *   *  multiplication
 *   /  division
 *
 * Examples:
 *   "5 + 3"
 *   "10 * 4"
 *   "20 / 5"
 *
 * The function returns dynamically allocated text.
 * The caller must free() the returned string.
 */
char *calculator_tool(const char *expression)
{
    double left_value;
    double right_value;
    double result;
    char operation;
    char response_buffer[INPUT_SIZE];

    if (expression == NULL) {
        return safe_strdup("Calculator error: no expression was provided.");
    }

    /*
     * sscanf() attempts to read:
     * - a floating-point number
     * - one operator character
     * - another floating-point number
     *
     * It returns the number of successfully parsed values.
     * We need all 3 values for a valid expression.
     */
    if (sscanf(expression, "%lf %c %lf",
               &left_value, &operation, &right_value) != 3) {
        return safe_strdup(
            "Calculator error: use the format: calc number operator number\n"
            "Example: calc 5 + 3"
        );
    }

    switch (operation) {
        case '+':
            result = left_value + right_value;
            break;

        case '-':
            result = left_value - right_value;
            break;

        case '*':
            result = left_value * right_value;
            break;

        case '/':
            /*
             * Division by zero must be checked before dividing.
             */
            if (right_value == 0.0) {
                return safe_strdup(
                    "Calculator error: division by zero is not allowed."
                );
            }

            result = left_value / right_value;
            break;

        default:
            return safe_strdup(
                "Calculator error: supported operators are +, -, *, and /."
            );
    }

    /*
     * %.2f prints the result with two digits after the decimal point.
     */
    snprintf(response_buffer,
             sizeof(response_buffer),
             "Calculator result: %.2f %c %.2f = %.2f",
             left_value,
             operation,
             right_value,
             result);

    return safe_strdup(response_buffer);
}

/*
 * add_turn()
 *
 * Saves a user message and its response in the rolling history.
 *
 * If the history already contains MAX_HISTORY turns:
 * 1. Free the oldest turn's allocated strings.
 * 2. Shift the remaining turns one position toward the beginning.
 * 3. Store the new turn in the final position.
 */
void add_turn(ConversationHistory *history,
              const char *user_input,
              const char *response)
{
    int i;
    int insert_index;
    char *user_copy;
    char *response_copy;

    if (history == NULL || user_input == NULL || response == NULL) {
        return;
    }

    /*
     * Make independent heap copies before changing history.
     * This lets the caller safely free its own response later.
     */
    user_copy = safe_strdup(user_input);
    response_copy = safe_strdup(response);

    /*
     * If either allocation fails, free anything that succeeded.
     * Then return without modifying history.
     */
    if (user_copy == NULL || response_copy == NULL) {
        free(user_copy);
        free(response_copy);

        fprintf(stderr, "Warning: could not save conversation history.\n");
        return;
    }

    if (history->count == MAX_HISTORY) {
        /*
         * History is full, so remove the oldest turn first.
         */
        free(history->turns[0].user);
        free(history->turns[0].response);

        /*
         * Shift turns [1..4] to [0..3].
         */
        for (i = 1; i < MAX_HISTORY; i++) {
            history->turns[i - 1] = history->turns[i];
        }

        /*
         * The new turn belongs at the last valid index.
         */
        insert_index = MAX_HISTORY - 1;
    } else {
        /*
         * History is not full, so add to the next empty location.
         */
        insert_index = history->count;
        history->count++;
    }

    history->turns[insert_index].user = user_copy;
    history->turns[insert_index].response = response_copy;
}

/*
 * print_history()
 *
 * Displays the current rolling conversation history.
 * This is optional for the agent's behavior, but useful for demonstrating
 * that only the last five turns are being kept.
 */
void print_history(const ConversationHistory *history)
{
    int i;

    if (history == NULL || history->count == 0) {
        printf("\n[Conversation history is empty]\n");
        return;
    }

    printf("\n--- Last %d conversation turn(s) ---\n", history->count);

    for (i = 0; i < history->count; i++) {
        printf("User: %s\n", history->turns[i].user);
        printf("Agent: %s\n", history->turns[i].response);
        printf("\n");
    }

    printf("-------------------------------------\n");
}

/*
 * free_history()
 *
 * Frees every dynamically allocated string still stored in the history.
 *
 * This must be called before the program exits so the program does not
 * leak memory.
 */
void free_history(ConversationHistory *history)
{
    int i;

    if (history == NULL) {
        return;
    }

    for (i = 0; i < history->count; i++) {
        free(history->turns[i].user);
        free(history->turns[i].response);

        /*
         * Setting pointers to NULL is a good defensive habit.
         */
        history->turns[i].user = NULL;
        history->turns[i].response = NULL;
    }

    history->count = 0;
}

/*
 * starts_with_calc()
 *
 * Returns 1 if the input begins with "calc" followed by:
 * - end of string, or
 * - whitespace
 *
 * This avoids treating words like "calculate" as calculator commands.
 */
int starts_with_calc(const char *input)
{
    if (input == NULL) {
        return 0;
    }

    if (strncmp(input, "calc", 4) != 0) {
        return 0;
    }

    return input[4] == '\0' || isspace((unsigned char)input[4]);
}

/*
 * main()
 *
 * This is the central agent loop:
 *
 * 1. Read input.
 * 2. Exit if the user typed "exit".
 * 3. Decide whether input is a calculator command.
 * 4. Call calculator_tool() or mock_model().
 * 5. Print the response.
 * 6. Save the turn in history.
 * 7. Free temporary response memory.
 * 8. Repeat.
 */
int main(void)
{
    char input[INPUT_SIZE];
    char *response;
    ConversationHistory history = {0};

    printf("Minimal C LLM Agent Harness\n");
    printf("Type a message, use \"calc 5 + 3\", or type \"exit\" to quit.\n");
    printf("Type \"history\" to view the saved conversation turns.\n\n");

    while (1) {
        printf("You> ");

        /*
         * fgets() returns NULL if it reaches end-of-file or encounters
         * an input error. In that case, leave the loop safely.
         */
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nInput ended. Exiting safely.\n");
            break;
        }

        remove_newline(input);

        /*
         * Ignore empty input lines so we do not create unnecessary turns.
         */
        if (input[0] == '\0') {
            continue;
        }

        /*
         * The exit command ends the main loop.
         * strcmp() returns 0 when both strings are identical.
         */
        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        /*
         * "history" is a small debugging command that displays the
         * currently stored rolling history.
         */
        if (strcmp(input, "history") == 0) {
            print_history(&history);
            continue;
        }

        /*
         * Route calculator commands to the tool.
         *
         * The text after "calc" is the expression. Leading spaces are
         * skipped for convenience.
         */
        if (starts_with_calc(input)) {
            const char *expression = input + 4;

            while (isspace((unsigned char)*expression)) {
                expression++;
            }

            response = calculator_tool(expression);
        } else {
            /*
             * All normal text goes to the mock LLM.
             */
            response = mock_model(input);
        }

        /*
         * Check whether dynamic memory allocation failed.
         */
        if (response == NULL) {
            fprintf(stderr, "Error: unable to allocate response memory.\n");
            continue;
        }

        printf("Agent> %s\n", response);

        /*
         * Save copies of both strings in history before freeing response.
         */
        add_turn(&history, input, response);

        /*
         * response was allocated by mock_model() or calculator_tool().
         * It is only temporary after add_turn() has copied it.
         */
        free(response);
    }

    /*
     * Free all strings still kept in the rolling history.
     */
    free_history(&history);

    return 0;
}