# ECE 309 Project 1 Vibe Coding Log

## Architecture and SDD Rules

Before generating the code, I defined the following requirements:

- The program must be written in standard C.
- The program runs in a terminal loop.
- Input is read using fgets().
- Typing "exit" safely terminates the program.
- A mock_model() function simulates an LLM.
- The harness stores only the five most recent conversation turns.
- Old history entries are safely removed when the limit is exceeded.
- Calculator requests are routed to a separate tool function.
- All dynamically allocated memory must be freed.
- No external libraries are used.

## Prompt 1 - Initial Harness

Paste the exact prompt you used to generate harness.c here.

## Result

The AI generated a single-file C implementation containing:
- terminal input handling
- mock model responses
- conversation history
- calculator tool execution
- memory management

## Prompt 2 - History Feature

Paste the prompt you used to add the history command here.

## Result

The program was updated so that typing "history" prints the currently stored conversation turns.

## Prompt 3 - Automated Testing

Paste the exact prompt used to generate test.sh here.

## Compilation

I compiled the program using:

gcc -Wall -Wextra -pedantic harness.c -o harness

The program compiled without warnings or errors.

## Manual Testing

I manually tested:
- hello
- normal text input
- calculator commands
- invalid calculator input
- history
- exit

## Automated Testing

I ran:

bash test.sh

Output:

PASS: Greeting response appeared.
PASS: Calculator response appeared.
PASS: History command printed 5 stored turns.
PASS: Oldest turn was removed from the rolling history.
PASS: Newest message appeared in history.
PASS: Program exited safely.
PASS: All automated tests passed.

## Memory Testing

I compiled using AddressSanitizer:

gcc -Wall -Wextra -fsanitize=address harness.c -o harness

I then ran:

printf "hello\ncalc 5 + 3\nexit\n" | ./harness

No AddressSanitizer errors were reported.