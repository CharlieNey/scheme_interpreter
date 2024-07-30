#!/usr/bin/env python3

import os
import re
import subprocess
import collections
import signal

TestResult = collections.namedtuple('TestResult', ['output', 'error'])
timed_out_string = "Timed out"

def get_student_output(executable_command, test_path) -> str:
    try:
        with open(test_path, 'r') as input_file:
            student_process = subprocess.run(
                executable_command,
                stdin=input_file,
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE,
                timeout=10)
        if (student_process.returncode == -signal.SIGSEGV):
            return "Segmentation fault"
        else:
            return student_process.stdout.decode('utf-8')
    except subprocess.TimeoutExpired:
        return timed_out_string


def get_correct_output(test_path) -> str:
    '''Gets correct output.'''
    with open(test_path, 'r') as correct_output:
        return correct_output.read()

def verify_ends_in_new_line(output: str) -> bool:
    '''Make sure that last line ends in a newline'''

    # If the output is completely empty, there are no lines at all, so
    # there's no newline to be found. The standard is that every line
    # ends in a newline, but in that case, there's no line.

    if len(output) >= 1:
        return output[-1] in ['\n', '\r']
    else:
        return True


def clean_output(output: str) -> str:
    '''Clean up output as much as possible to allow the student output and the
    correct output to be compared.'''
    result = output

    # If any line starts off with an error phrase, truncate the line just to
    # include the error. This is for test matching purposes.
    result = re.sub('^Syntax error.*$', 'Syntax error',
                    result,
                    flags=re.MULTILINE | re.IGNORECASE)
    result = re.sub('^Evaluation error.*$',
                    'Evaluation error',
                    result,
                    flags=re.MULTILINE | re.IGNORECASE)
    result = re.sub('^Evaluation error.*$',
                    'Evaluation error',
                    result,
                    flags=re.MULTILINE | re.IGNORECASE)

    # Sequences of one or more whitespace
    result = re.sub('\\s+', ' ', result)

    # Parens followed by whitespace should just remove whitespace
    result = re.sub('\(\\s+', '(', result)

    # Whitespace followed by right paren should just remove whitespace
    result = re.sub('\\s+\)', ')', result)

    # Trailing zeros after a decimal at end, i.e. 1.0000 -> 1
    result = re.sub('\\.0+$', '', result)

    # Trailing decimal places after non-zero values,
    # followed by whitespace i.e. 1.32000 -> 1.32
    result = re.sub('(\\.)([1-9]+)(0*)\\s+', '\\1\\2 ', result)

    # Trailing decimal places after non-zero values, at end,
    # i.e. 1.32000 -> 1.32
    result = re.sub('(\\.)([1-9]+)(0*)$', '\\1\\2', result)

    result = (result
              .lstrip()   # Whitespace at beginning
              .rstrip())  # Whitespace at end

    return result


def run_tests_with_valgrind(executable_command, test_path) -> TestResult:
    '''Run again with valgrind (just student version) and look for errors)'''
    valgrind_command = \
        'ulimit -c 0 && ' + \
        'valgrind ' + \
        '--leak-check=full ' + \
        '--show-leak-kinds=all ' + \
        '--errors-for-leak-kinds=all ' + \
        '--error-exitcode=99 ' + \
        executable_command
    with open(test_path, 'r') as input_file:
    
        try:
            process = subprocess.run(
                valgrind_command,
                stdin=input_file,
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE,
                timeout=10,
                shell=True)

            # Verify that there is no error from Valgrind. This is tough to do
            # based on error code, because if Valgrind has no error, it passes
            # through the error code from the original program, which may have
            # a legitimate error in the case of bad input (i.e., bad Scheme code).
            # Therefore, test for error by looking for the presence of the string
            # "0 errors from 0 contexts" which is that Valgrind prints when
            # no errors.
            valgrind_error_location = (
                process.stdout
                .decode('utf-8')
                .find("ERROR SUMMARY: 0 errors from 0 contexts")
            )
            valgrind_error_found = valgrind_error_location == -1
            return TestResult(process.stdout.decode('utf-8'),
                              valgrind_error_found)
        except subprocess.TimeoutExpired:
            return TestResult(timed_out_string, True)


def runcmd(cmd, input_text=None):
    splitcmd = cmd.split()
    return subprocess.run(splitcmd, input=input_text,
                          stderr=subprocess.STDOUT,
                          stdout=subprocess.PIPE, encoding='utf-8')


def buildCode() -> int:

    # Make sure that warnings aren't suppressed
    for filename in os.listdir():
        split_filename = os.path.splitext(filename)
        if split_filename[1] in ['.c', '.h']:
            with open(filename, 'r') as f:
                for line in f.readlines():
                    if 'diagnostic' in line and 'ignore' in line:
                        return 'Please do not disable warnings.'

    # Compile, show output
    compile_return = runcmd('make')
    print(compile_return.stdout)

    return_code = compile_return.returncode

    # Make sure that warning causes test to fail
    if ('warning' in compile_return.stdout):
        print('Test failed because of compiler warning.')
        return_code = 1

    return return_code


def runIt(test_dir, valgrind=True) -> None:

    returncode = buildCode()
    print('return code is ', returncode)
    if returncode != 0:
        return returncode

    error_encountered = False
    executable_command = "./interpreter"

    test_names = [test_name.split('.')[0]
                  for test_name in sorted(os.listdir(test_dir))
                  if test_name.split('.')[1] == 'scm']

    for test_name in test_names:
        print('------Test', test_name, '------')

        test_input_path = os.path.join(test_dir, test_name + ".scm")
        test_output_path = os.path.join(test_dir, test_name + ".output")
        student_raw_output = get_student_output(executable_command,
                                            test_input_path)
        student_output = clean_output(student_raw_output)

        correct_raw_output = get_correct_output(test_output_path)
        correct_output = clean_output(correct_raw_output)

        if not verify_ends_in_new_line(student_raw_output):
            error_encountered = True
            print("---OUTPUT INCORRECT---")
            print("Output does not end in a newline.")
            print("Student (raw) output:")
            print(repr(student_raw_output))
            print('Correct (raw) output:')
            print(repr(correct_raw_output))

        elif student_output != correct_output:
            error_encountered = True
            print("---OUTPUT INCORRECT---")
            print('Correct output:')
            print(correct_output)
            print('Student output:')
            print(student_output)
        else:
            print("---OUTPUT CORRECT---")

        if valgrind and student_output != timed_out_string:
            valgrind_test_results = run_tests_with_valgrind(
                executable_command,
                test_input_path)

            if valgrind_test_results.error:
                error_encountered = True
                print('---VALGRIND ERROR---')
                print('Valgrind test results')
                print(valgrind_test_results.output)
            else:
                print('---VALGRIND NO ERROR---')

    return error_encountered
