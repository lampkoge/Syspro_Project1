#!/bin/bash

# Stops queued jobs first, cause if i remove the running jobs first without changing the concurrency to 0,
# it runs the queued jobs (as it should :) ) so it needs to be executed more times

# Get jobs using ./jobCommander poll running/queued
# The poll command returns strings like "<jobID, job, position_in_queue>"
number=0
./jobCommander poll queued | while IFS= read -r line;
do
    if ((number > 1)); then
        iteration=0
        IFS=','
        read -ra ADDR <<< "$line"
        if [[ "$line" != "Queue has no elements" ]]; then
            for i in "$ADDR";
            do
                if [[ $iteration == 0 ]]; then
                    jobID=${i:1}
                    if [[ "$jobID" == ""  || "$jobID" == "Queue has no elements" ]]; then
                        continue;
                    else
                        ./jobCommander stop "$jobID"
                    fi
                fi
                iteration=$((iteration+1))
            done
        fi
    fi
    number=$((number+1))
done
number=0
./jobCommander poll running | while IFS= read -r line;
do
    if (($number > 1)); then
        iteration=0
        # will split on ','
        IFS=','
        read -ra ADDR <<< "$line"
        if [[ "$line" != "Queue has no elements" ]]; then
            for i in "$ADDR";
            do
                # we just need the jobID (the first token)
                if [[ $iteration == 0 ]]; then
                    # remove the "<"
                    jobID=${i:1}
                    if [[ "$jobID" == "" || "$jobID" == "Queue has no elements" ]]; then
                        continue;
                    else
                        ./jobCommander stop "$jobID"
                    fi
                fi
                iteration=$((iteration+1))
            done
        fi
    fi
    number=$((number+1))
done