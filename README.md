# ECE 309 Project 1 - LLM Mini Harness

This project implements a minimal LLM agent harness in C.

## Features

- Terminal-based user interaction
- Mock LLM responses
- Five-turn conversation history
- Calculator tool execution
- Safe program shutdown
- Automated Bash testing

## Compilation

gcc -Wall -Wextra -pedantic harness.c -o harness

## Running

./harness

## Example Commands

hello

calc 5 + 3

history

exit

## Automated Testing

bash test.sh

## Memory Testing

gcc -Wall -Wextra -fsanitize=address harness.c -o harness