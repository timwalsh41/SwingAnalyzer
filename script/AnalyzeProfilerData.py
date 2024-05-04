import os
import matplotlib.pyplot as plt

import pdb

# User-provided list of Profiler names
profiler_names = ['VideoChannelImageProfiler', 'VideoChannelFrameProfiler']

# Find the log file with the extension ".log" in the parent folder
parent_folder = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
log_file = next((file for file in os.listdir(parent_folder) if file.endswith(".log")), None)

if log_file:
    with open(os.path.join(parent_folder, log_file), "r") as file:
        lines = file.readlines()

        for profiler_name in profiler_names:
            fig,ax = plt.subplots(2,1)

            interval_data = []
            duration_data = []

            interval_string = f'Profiler {profiler_name} interval data'
            duration_string = f'Profiler {profiler_name} duration data'

            print(f'Analyzing profiler {profiler_name}')

            line_index = 0
            while line_index < len(lines):
                line = lines[line_index]
                if interval_string in line:
                    print('\tInterval data')
                    line_index += 1
                    dataline = lines[line_index]
                    while "Profile data" in dataline:
                        # Extract the floating point value from the line
                        interval_data.append(float(dataline.split()[-1]))
                        line_index += 1
                        dataline = lines[line_index]

                elif duration_string in line:
                    print('\tDuration data')
                    line_index += 1
                    dataline = lines[line_index]
                    while "Profile data" in dataline:
                        # Extract the floating point value from the line
                        duration_data.append(float(dataline.split()[-1]))
                        line_index += 1
                        dataline = lines[line_index]

                else:
                    line_index += 1

            # Create a histogram plot for interval data
            ax[0].hist(interval_data, bins=40)
            ax[0].set_ylabel("Interval")

            # Create a histogram plot for duration data
            ax[1].hist(duration_data, bins=40)
            ax[1].set_xlabel("Time (s)")
            ax[1].set_ylabel("Duration")
            ax[0].set_title(f"{profiler_name}")

    plt.show()
else:
    print("No log file found with the extension '.log' in the parent folder.")