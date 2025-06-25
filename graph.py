# Generated with Gemini A

import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

def plot_neuron_voltages(file_path="output.csv", spike_threshold=0.0):
    """
    Reads a CSV file where rows represent time steps (in ms) and all columns
    are voltage outputs for different neurons. Generates a raster plot.

    Args:
        file_path (str): The path to the CSV file.
                         Assumes each row is 1ms, and all columns are neuron voltages.
        spike_threshold (float): The voltage value above which a neuron is considered to have spiked.
                                 Defaults to 0.0 to plot all non-zero values.
    """
    if not os.path.exists(file_path):
        print(f"Error: The file '{file_path}' was not found.")
        print("Please make sure 'output.csv' is in the same directory as this script, or provide the full path.")
        return

    try:
        # Read the CSV file into a pandas DataFrame without assuming a time column
        df = pd.read_csv(file_path, header=None) # Assume no header, as all cols are neurons

        # Create the time axis based on the number of rows (each row is 1 ms)
        # Time starts at 0 ms for the first row
        time_column = np.arange(df.shape[0])

        # All columns now represent neuron voltages
        voltage_columns_df = df

        # Ensure there is at least one neuron column
        if voltage_columns_df.shape[1] == 0:
            print("Error: The CSV file must contain at least one column of neuron voltage data.")
            return

        # Create the plot
        plt.figure(figsize=(12, 8)) # Adjust figure size for better readability of many neurons

        # Prepare data for raster plot
        event_times_list = [] # Will contain a list of spike times for each neuron
        neuron_labels_full = []   # Will contain the labels for the y-axis

        # Iterate through each neuron column by its index
        for neuron_idx, col_name in enumerate(voltage_columns_df.columns):
            # Get the voltage series for the current neuron
            neuron_voltages = voltage_columns_df[col_name]
            
            # Find time points where voltage exceeds the threshold
            # .tolist() converts the pandas Series of spike times to a standard Python list
            spike_times = time_column[neuron_voltages > spike_threshold].tolist()
            
            if spike_times: # Only add to the list if spikes are detected for this neuron
                event_times_list.append(spike_times)
                neuron_labels_full.append(str(neuron_idx)) # Label neurons by their 0-based index
        
        if not event_times_list:
            print(f"No spikes detected above the specified threshold ({spike_threshold}).")
            print("Consider lowering the 'spike_threshold' parameter if you expect spikes.")
            plt.title(f'No Spikes Detected (Threshold > {spike_threshold})')
            plt.xlabel('Time (ms)')
            plt.ylabel('Neuron ID')
            plt.show()
            return

        # Create the raster plot using plt.eventplot
        # Vertical bars for better resolution are achieved by linelengths.
        # Increased 'linewidth' for thicker lines/boxes.
        plt.eventplot(event_times_list, lineoffsets=np.arange(len(event_times_list)), 
                      linelengths=0.9, # Adjusted linelengths slightly for a more "boxy" look with large linewidth
                      color='black', alpha=0.8,
                      linewidth=10) # Significantly increased linewidth for thicker, box-like lines

        # Set y-axis labels to correspond to neuron indices (just numbers), showing every 100th label
        display_ticks = []
        display_labels = []
        tick_interval = 100 # Label every 100th neuron
        for idx, label in enumerate(neuron_labels_full):
            if idx % tick_interval == 0:
                display_ticks.append(idx)
                display_labels.append(label)
        
        plt.yticks(display_ticks, display_labels)
        
        # Add titles and Y-axis label. X-axis label is already "Time (ms)"
        plt.title('Neuron Spiking Raster Plot')
        plt.xlabel('Time (ms)')
        plt.ylabel('Neuron ID') 
        plt.grid(True, axis='x', linestyle='--', alpha=0.7) # Add grid lines for time
        
        # Adjust y-limits to ensure markers are clearly visible and centered
        plt.ylim(-0.5, len(neuron_labels_full) - 0.5) 

        plt.tight_layout() # Adjust layout to prevent labels from overlapping
        plt.show()

    except Exception as e:
        print(f"An error occurred while processing the CSV file: {e}")
        print("Please ensure the CSV file is correctly formatted with numerical data and no unexpected characters.")

if __name__ == "__main__":
    # Call the function to plot the raster plot
    # You can change the 'spike_threshold' here if your non-zero definition
    # is actually a specific voltage, e.g., plot_neuron_voltages("output.csv", spike_threshold=29.0)
    plot_neuron_voltages("output.csv", spike_threshold=0.0)

