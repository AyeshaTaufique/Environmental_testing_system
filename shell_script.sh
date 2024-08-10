#!/bin/bash

cd ~/Desktop
gcc try.c report_email_sender.c cJSON.c -o test -lcurl

if [ $? -eq 0 ]; then
 

  while true; do
    # Run the compiled program
    ./test
    exit_code=$?

    # Check the exit code of the program
    if [ $exit_code -eq 0 ]; then
      echo "Program executed successfully."
      echo " "
    else
      echo "Program exited with an error. Exit code: $exit_code"
    fi

    # Wait for the next iteration
    echo "Waiting for the next iteration..."
    echo " "
    sleep 10
  done
else
  echo "Compilation failed. Please check the errors."
fi
