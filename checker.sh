#!/bin/bash
# Set the directory path
directory="qcir"

# Iterate over files in the directory
> ".vscode/final.log"
for file in "$directory"/*; do
    # Check if the file size is smaller than 100 Mb
    # if [ $(stat -f "%z" "$file") -lt 104857600 ]; then
        # Call the generated program with the file
        # Run qcir.exe with the specified options and redirect output to a file
        build/qcir.exe -cleansed -light "$file" > ".vscode/a.log" 2>&1

        code=$?
        echo "Return code for file: $file is $code" >> ".vscode/final.log"
        echo "Return code for file: $file is $code"
        # Check the return code
        if [ $code -ne 0 ]; then
            echo "Return code is not 0 for file: $file" >> ".vscode/final.log"
        fi
    # fi
done
