import subprocess
import os
import re

current_directory = os.path.dirname(os.path.abspath(__file__))

EXECUTABLE_PATH = os.path.join(current_directory, "pizza")

OUTPUT_FILE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "output.txt")

num_requests = 53

def run_pizza_simulation():
    args = ["5"]
    for i in range(11):
        args.append(f'customer.in{i}')
    command = [EXECUTABLE_PATH] + args
    try:
        with open(OUTPUT_FILE_PATH, 'w') as output_file:
            result = subprocess.run(command, stdout=output_file, stderr=subprocess.PIPE, text=True)
        return result.stderr
    except Exception as e:
        print(f"Error running pizza simulation: {e}")
        return None

def analyze_output():
    payment_regex = r"customer (\d+) pays driver (\d+)"
    request_regex = r"customer (\d+) requests pizza"
    customer_requests = {}
    customer_payments = {}
    failed = False
    
    output = ""

    try:
        with open(OUTPUT_FILE_PATH, 'r') as file:
            for line in file:
                output += line
                # Track customer requests
                if "requests pizza" in line:
                    match = re.search(request_regex, line)
                    if match:
                        customer_id = match.group(1)
                        customer_requests[customer_id] = customer_requests.get(customer_id, 0) + 1

                # Track customer payments
                if "pays driver" in line:
                    match = re.search(payment_regex, line)
                    if match:
                        customer_id = match.group(1)
                        customer_payments[customer_id] = customer_payments.get(customer_id, 0) + 1

                # Check if all reqeusts are finished
                if "All customers are done" in line:
                    print("Program finished")

        total_payments = 0
        # Compare customer requests and payments
        for customer_id, request_count in customer_requests.items():
            payment_count = customer_payments.get(customer_id, 0)
            total_payments += payment_count
            if payment_count != request_count:
                print(f"Customer {customer_id} has {payment_count} payments but requested pizza {request_count} times.")
                failed = True
        # Check total number of payments
        if total_payments != num_requests:
            print(f"FAILED: Not all pizzas were payed for! {total_payments} payments but {num_requests} expected")
            failed = True
    except Exception as e:
        print(e)

    if failed:
        print(output)
    return failed

def main():
    failed_tests = 0
    for i in range(1000):
        print(f"Running test {i}")
        stderr = run_pizza_simulation()

        if stderr:
            print("Error in simulation:", stderr)
            return

        test_failed = analyze_output()
        if test_failed:
            failed_tests += 1

        print("\n")

    if failed_tests:
        print(f"Failed {failed_tests} tests")
    else:
        print("Tests passed successfully!")

if __name__ == "__main__":
    main()