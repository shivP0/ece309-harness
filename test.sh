#!/bin/bash

# test.sh
#
# Simple automated test script for the compiled ./harness program.
#
# It sends several lines of input to the program, captures its output,
# then checks whether expected text appears.
#
# Run:
#   chmod +x test.sh
#   ./test.sh

# Stop the script if a command fails unexpectedly.
set -e

# Make sure the compiled program exists and can be executed.
if [ ! -x "./harness" ]; then
    echo "FAIL: ./harness was not found or is not executable."
    echo "Compile your program first:"
    echo "  gcc -Wall -Wextra -pedantic -std=c11 harness.c -o harness"
    exit 1
fi

# Create a temporary file to store all program output.
OUTPUT_FILE=$(mktemp)

# Make sure the temporary file is deleted when the script finishes,
# whether the test passes or fails.
trap 'rm -f "$OUTPUT_FILE"' EXIT

# Send test input to the program.
#
# The messages after the calculator command create more than 5 turns.
# Then "history" asks the harness to print its rolling history.
# Finally, "exit" cleanly ends the harness program.
printf '%s\n' \
    "hello" \
    "calc 5 + 3" \
    "first normal message" \
    "second normal message" \
    "third normal message" \
    "fourth normal message" \
    "history" \
    "exit" | ./harness > "$OUTPUT_FILE"

# Keep track of whether every test passes.
ALL_TESTS_PASSED=true

# Test 1: Check for the hardcoded greeting from mock_model().
if grep -Fq "Hello! I am a mock LLM running inside a simple C agent harness." \
    "$OUTPUT_FILE"; then
    echo "PASS: Greeting response appeared."
else
    echo "FAIL: Greeting response did not appear."
    ALL_TESTS_PASSED=false
fi

# Test 2: Check for the calculator result.
#
# Your C program prints numbers with two decimal places.
if grep -Fq "Calculator result: 5.00 + 3.00 = 8.00" \
    "$OUTPUT_FILE"; then
    echo "PASS: Calculator response appeared."
else
    echo "FAIL: Calculator response did not appear."
    ALL_TESTS_PASSED=false
fi

# Test 3: Check that the history command printed a history heading.
if grep -Fq -- "--- Last 5 conversation turn(s) ---" "$OUTPUT_FILE"; then
    echo "PASS: History command printed 5 stored turns."
else
    echo "FAIL: History output did not show 5 conversation turns."
    ALL_TESTS_PASSED=false
fi

# Test 4: The first message should be removed once more than five
# user/model turns have been saved. "hello" is the oldest turn here.
if grep -Fq "User: hello" "$OUTPUT_FILE"; then
    echo "FAIL: Oldest turn was still present in the 5-turn history."
    ALL_TESTS_PASSED=false
else
    echo "PASS: Oldest turn was removed from the rolling history."
fi

# Test 5: Check that the newest normal message appears in history.
if grep -Fq "User: fourth normal message" "$OUTPUT_FILE"; then
    echo "PASS: Newest message appeared in history."
else
    echo "FAIL: Newest message did not appear in history."
    ALL_TESTS_PASSED=false
fi

# Test 6: Check that the program reached its normal exit message.
if grep -Fq "Goodbye!" "$OUTPUT_FILE"; then
    echo "PASS: Program exited safely."
else
    echo "FAIL: Program did not print its exit message."
    ALL_TESTS_PASSED=false
fi

# Print one final result for the entire script.
if [ "$ALL_TESTS_PASSED" = true ]; then
    echo "PASS: All automated tests passed."
    exit 0
else
    echo "FAIL: One or more automated tests failed."
    echo
    echo "Captured program output:"
    echo "----------------------------------------"
    cat "$OUTPUT_FILE"
    echo "----------------------------------------"
    exit 1
fi