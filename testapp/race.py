import subprocess
import os

# Get the directory of the current script
current_directory = os.path.dirname(os.path.abspath(__file__))

EXECUTABLE_PATH = os.path.join(current_directory, "race")

for i in range(10):
    try:
        # Run the executable
        result = subprocess.run(
            [EXECUTABLE_PATH],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        print("Output:", result.stdout.decode())
    except subprocess.CalledProcessError as e:
        print("What")

print("Finished running the executable.")