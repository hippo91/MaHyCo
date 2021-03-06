#!/bin/bash
# Launch the test and compares obtained results to expected ones
set -euo pipefail 

# This function launch the computation by calling the executable with arguments
# taken from args.txt file
function launch_computation {
  local readonly exe_path=$1
  local readonly data_dir=$2
  local return_code=0
  OMP_NUM_THREADS=3 $1 $data_dir/donnees.txt
  if [[ $? -ne 0 ]]; then
    echo "A problem occured during test execution."
    return_code=1
  fi
  return ${return_code}
}

# This function compares the results obtained in the output directory with those
# expected in the reference directory
function compare_results {
  local readonly reference_dir=$1
  local return_code=0
  if ! diff -r output "$reference_dir/reference" > /dev/null 2>&1; then
    echo "Test failure! Output is different from reference"
    return_code=1
  fi
  return ${return_code}
}

# Main function. Calls launch_computation and compare_results
# For each test a directory is created inside /tmp
function main {
  local readonly exe_path=$1
  local readonly test_dir=$2

  echo "Executable path : ${exe_path}"
  echo "Executing test $(basename ${test_dir})"

  tmp_dir=$(mktemp -d -t mahyco-ci-XXXXXXXXXX)

  if [[ ! -d ${tmp_dir} ]]; then
    echo "Unable to create a temporary directory!"
    echo "Aborting!"
    exit 3
  fi

  cd ${tmp_dir}
  echo "This directory contains the output of the test under ${test_dir}" > README.txt

  launch_computation ${exe_path} ${test_dir}
  if [[ $? -ne 0 ]]; then
    echo "Aborting!"
    exit 1
  fi

  compare_results ${test_dir}
  if [[ $? -ne 0 ]]; then
    echo "Aborting!"
    exit 2
  else
    echo "Success!"
  fi
  exit 0
}

main $@
