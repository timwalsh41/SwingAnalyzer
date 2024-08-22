import os
import matplotlib.pyplot as plt

import pdb

# User-provided list of Profiler names
profiler_names = ['leftVideoChannelImageProfiler', 'leftVideoChannelFrameProfiler',
                  'rightVideoChannelImageProfiler', 'rightVideoChannelFrameProfiler']

# Find the log file with the extension ".log" in the parent folder
parent_folder = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
log_file = next((file for file in os.listdir(parent_folder) if file.endswith(".log")), None)

if log_file:
    print(f"Analyzing log file: {log_file}")
    with open(os.path.join(parent_folder, log_file), "r") as file:
        lines = file.readlines()

        for profiler_name in profiler_names:
            fig,ax = plt.subplots(2,1)

            interval_data = []
            duration_data = []

            interval_string = f'Profiler {profiler_name} interval data'
            duration_string = f'Profiler {profiler_name} duration data'

            line_index = 0
            while line_index < len(lines):
                line = lines[line_index]
                if interval_string in line:
                    line_index += 1
                    dataline = lines[line_index]
                    while "Profile data" in dataline:
                        # Extract the floating point value from the line
                        interval_data.append(float(dataline.split()[-1])*1000)
                        line_index += 1
                        dataline = lines[line_index]

                elif duration_string in line:
                    line_index += 1
                    dataline = lines[line_index]
                    while "Profile data" in dataline:
                        # Extract the floating point value from the line
                        duration_data.append(float(dataline.split()[-1])*1000)
                        line_index += 1
                        dataline = lines[line_index]

                else:
                    line_index += 1

            # Create a histogram plot for interval data
            hist_range = [min(interval_data), max(interval_data)]
            ax[0].hist(interval_data, bins=round(len(interval_data)/10))
            ax[0].set_ylabel("Interval")

            # Create a histogram plot for duration data
            hist_range = [min(duration_data), max(duration_data)]
            ax[1].hist(duration_data, bins=round(len(duration_data)/10))
            ax[1].set_xlabel("Time (ms)")
            ax[1].set_ylabel("Duration")
            ax[0].set_title(f"{profiler_name}")

            # add statistics to plots
            ax[0].text(0.5, 0.5, f"Mean: {sum(interval_data)/len(interval_data)}\nMin: {min(interval_data)}\nMax: {max(interval_data)}\nStd: {sum([(x - sum(interval_data)/len(interval_data))**2 for x in interval_data])/(len(interval_data)-1)}", horizontalalignment='center', verticalalignment='center', transform=ax[0].transAxes)
            ax[1].text(0.5, 0.5, f"Mean: {sum(duration_data)/len(duration_data)}\nMin: {min(duration_data)}\nMax: {max(duration_data)}\nStd: {sum([(x - sum(duration_data)/len(duration_data))**2 for x in duration_data])/(len(duration_data)-1)}", horizontalalignment='center', verticalalignment='center', transform=ax[1].transAxes)
            
    plt.show()
else:
    print("No log file found with the extension '.log' in the parent folder.")