#!/bin/bash

# Get the list of branches to check. Use default if no branch specified.
if [ "$#" -ne "0" ]
then
  branches="$@"
else
  branches="default"
fi

# Create a logfile based on the current time
timestamp=`eval date +%d_%m_%Y_%R:%S`
logfile="/tmp/gazebo_test-$timestamp.txt"

# Process each branch from the command line
for branch in $branches
do
  cd 

  # Create working directory
  rm -rf /tmp/gazebo_build
  mkdir /tmp/gazebo_build

  # Clone
  hg clone https://bitbucket.org/osrf/gazebo /tmp/gazebo_build

  # Get the correct branch
  cd /tmp/gazebo_build
  hg up $branch

  # Build
  mkdir build
  cd build
  cmake ../
  make -j4

  echo "** Branch: $branch **" >> $logfile

  # Run make test many times, only capture failures
  for i in {1..100}
  do
    make test | grep -i "fail" >> $logfile 
  done

  # Run code checker
  cd /tmp/gazebo_build/
  sh tools/code_check.sh >> $logfile

  # Cleanup
  cd
  rm -rf /tmp/gazebo_build
done
