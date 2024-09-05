#!/bin/bash

# Define the compiled program and test variables
PROGRAM="./ft_ping"
TEST_LOG="test_output.log"

# Function to run a test case and check its output
run_test() {
    local description=$1   # Description of the test case
    local command=$2       # Command to run
    local expected=$3      # Expected output substring

    # Print Running test: and the test case description and command in yellow
    echo -e "\e[33mRunning test: $description\e[0m"
    echo -e "\e[33m$command\e[0m"
    
    # Run the command and capture the output
    $command > $TEST_LOG 2>&1
    local result=$?

    #ignore last new line from test log
    sed -i -e '$ d' $TEST_LOG

    # print found and expected
    echo "Found:"
    cat $TEST_LOG
    echo "Expected:"
    echo $expected
    

    # Check if the command was successful and if the output matches the expected substring
    if [ $result -eq 0 ] && grep -q "$expected" $TEST_LOG; then
        # print test passed in green
        echo -e "\e[32mTest passed.\e[0m"
    else
        # print test failed in red
        echo -e "\e[31mTest failed.\e[0m"
        echo "Expected output to contain:" 
        echo $expected
        echo "Actual output:"
        cat $TEST_LOG
        diff -u <(echo "$expected") $TEST_LOG
        diff <(echo -n "${expected}" | xxd) <(echo -n "$(cat $TEST_LOG)" | xxd)
    fi
    echo
}

# Run the test cases with explicit newlines in the expected outputs
run_test "Test verbose mode" "$PROGRAM -v" '[INFO] Option -v selected'
run_test "Test help option" "$PROGRAM --help" $'Usage: program [options]\n'
run_test "Test multiple flags (-f, -l, -n)" "$PROGRAM -f -l -n" $'Option -f selected\n'
run_test "Test option -c with value" "$PROGRAM -c 5" $'Option -c with value \'5\' selected\n'
run_test "Test --ttl option" "$PROGRAM --ttl 64" $'Option --ttl with value \'64\' selected\n'
run_test "Test --ip-timestamp option" "$PROGRAM --ip-timestamp" $'Option --ip-timestamp selected\n'
run_test "Test invalid flag" "$PROGRAM -x" $'Usage: program [options]\n'

# Clean up
rm -f $TEST_LOG

echo "All tests completed."
