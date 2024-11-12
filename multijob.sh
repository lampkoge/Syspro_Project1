#!/bin/bash

# Takes as input files containing jobs to be executed (one per line). 

# Get number of arguments passed 
counter=$#
if [[ $counter -eq $zero ]]; then
    echo "No files provided !"
    exit 1
fi

# for each file in arguments 
for file in "$@";
do
    # check if it exists
    if [ -f "$file" ]; then
        while read line; do
            ./jobCommander issueJob $line
        done < "$file"
    else
        echo "File does not exist : '$file' "
    fi
done