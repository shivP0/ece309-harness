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

----------------------------------------------------------------------------------------

## Prompt 1 - Initial Harness

I am building ECE 309 Project 1, a minimal LLM agent harness in C.

I am a beginner, so keep the implementation simple, readable, and heavily commented.

Requirements:

1. Use standard C and make the program compile with GCC in a POSIX environment.
2. Do not use any external libraries.
3. The program should run in a loop and read user input using fgets().
4. If the user enters "exit", the program should safely terminate.
5. Create a mock_model() function that simulates an LLM response.
6. If the input contains "hello", the mock model should return a hardcoded greeting.
7. Otherwise, the mock model may echo or respond to the user's message.
8. Maintain conversation history containing only the last 5 user/model turns.
9. Manage memory safely and free any dynamically allocated memory before the program exits.
10. Add a simple tool execution feature for mathematical calculations.
11. For simplicity, support commands such as:
    calc 5 + 3
    calc 10 * 4
    calc 20 / 5
    calc 7 - 2
12. The calculator should be implemented as a separate function.
13. The main harness should decide whether to call the calculator tool or the mock model.
14. Print the model/tool response to the terminal.
15. Add clear comments explaining the architecture and important C concepts.
16. Keep everything in one file named harness.c.

Before writing the code, briefly explain the program architecture and state machine. Then provide the complete harness.c file.


----------------------------------------------------------------------------------------


## Result

The AI generated a single-file C implementation containing:
- terminal input handling
- mock model responses
- conversation history
- calculator tool execution
- memory management

## Prompt 2 - History Feature

Modify my program so that if the user types "history", it prints the currently stored conversation history. The harness must still store only the last 5 user/model turns. Keep the implementation simple and memory safe.


## Result

The program was updated so that typing "history" prints the currently stored conversation turns.



## Prompt 3 - Automated Testing

I have a compiled C program named harness.

Create a simple Bash script named test.sh that automatically tests the program.

It should:
1. Send "hello" to the program.
2. Test at least one calculator command such as "calc 5 + 3".
3. Send several messages to exercise conversation history.
4. Send "exit" at the end.
5. Capture the program output.
6. Check that the greeting and calculator result appear.
7. Print PASS or FAIL messages.

Keep the Bash script simple and beginner-friendly.


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