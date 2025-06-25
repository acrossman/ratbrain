import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

def plot_neuron_voltages(file_path="output.csv", spike_threshold=0.0):
    """
    Reads a CSV file where rows represent time steps (in ms) and all columns
    are voltage outputs for different neurons. Generates a raster plot.
    It also marks the time points with the highest sum of spikes within
    every 100ms interval. Additionally, it plots the average voltage trace
    of all neurons in a separate subplot.

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
        total_simulation_time = df.shape[0]

        # All columns now represent neuron voltages
        voltage_columns_df = df

        # Ensure there is at least one neuron column
        if voltage_columns_df.shape[1] == 0:
            print("Error: The CSV file must contain at least one column of neuron voltage data.")
            return

        # Determine the total number of neurons from the input CSV columns
        total_neurons = voltage_columns_df.shape[1]

        # Create a figure with two subplots: one for the raster plot, one for the average voltage
        fig, axes = plt.subplots(nrows=2, ncols=1, figsize=(12, 10), gridspec_kw={'height_ratios': [3, 1]})
        
        # --- Raster Plot Subplot ---
        ax_raster = axes[0]

        # Prepare data for raster plot
        all_spike_data = [] # Will store (neuron_idx, [spike_times]) tuples

        # Iterate through each neuron column by its index (0 to total_neurons-1)
        # and identify spike times for the raster plot
        for neuron_idx in range(total_neurons):
            neuron_voltages = voltage_columns_df.iloc[:, neuron_idx]
            spike_times = time_column[neuron_voltages > spike_threshold].tolist()
            if spike_times:
                all_spike_data.append((neuron_idx, spike_times))

        if not all_spike_data:
            print(f"No spikes detected above the specified threshold ({spike_threshold}).")
            print("Consider lowering the 'spike_threshold' parameter if you expect spikes.")
            ax_raster.set_title(f'No Spikes Detected (Threshold > {spike_threshold})')
            ax_raster.set_xlabel('Time (ms)')
            ax_raster.set_ylabel('Neuron ID')
            # If no spikes, still set the y-axis ticks for context
            display_ticks = []
            display_labels = []
            tick_interval = 100
            for i in range(0, total_neurons, tick_interval):
                display_ticks.append(i)
                display_labels.append(str(i))
            if total_neurons > 0 and (total_neurons - 1) % tick_interval != 0:
                if (total_neurons - 1) not in display_ticks: 
                    display_ticks.append(total_neurons - 1)
                    display_labels.append(str(total_neurons - 1))
                combined_sorted = sorted(zip(display_ticks, display_labels))
                display_ticks = [t for t, l in combined_sorted]
                display_labels = [l for t, l in combined_sorted]
            ax_raster.set_yticks(display_ticks, display_labels)
            ax_raster.set_ylim(-0.5, total_neurons - 0.5)
            # Proceed to plot average neuron voltage even if no spikes overall
        else:
            # Separate spike times and their corresponding neuron IDs for plt.eventplot
            event_times_list = [item[1] for item in all_spike_data]
            actual_line_offsets = [item[0] for item in all_spike_data] 

            # Create the raster plot using plt.eventplot
            ax_raster.eventplot(event_times_list, lineoffsets=actual_line_offsets, 
                                  linelengths=0.9, 
                                  color='black', alpha=0.8,
                                  linewidth=10)

            # --- Logic to find highest sum of spikes every 100 ms for raster plot ---
            total_spikes_per_ms = np.zeros(total_simulation_time)
            for neuron_idx, spike_times in all_spike_data:
                for t in spike_times:
                    if 0 <= t < total_simulation_time:
                        total_spikes_per_ms[int(t)] += 1

            peak_spike_times = []
            window_size_ms = 100

            for start_time in range(0, total_simulation_time, window_size_ms):
                end_time = min(start_time + window_size_ms, total_simulation_time)
                
                window_spikes = total_spikes_per_ms[start_time:end_time]
                
                if len(window_spikes) > 0 and np.max(window_spikes) > 0:
                    max_index_in_window = np.argmax(window_spikes)
                    peak_time = start_time + max_index_in_window
                    peak_spike_times.append(peak_time)

            # Mark the highest sum of spikes every 100 ms with vertical lines
            for peak_time in peak_spike_times:
                ax_raster.axvline(x=peak_time, color='red', linestyle='--', linewidth=1.5, alpha=0.7)
            
            # Add a single legend entry for the peak lines
            if peak_spike_times:
                ax_raster.plot([], [], color='red', linestyle='--', linewidth=1.5, alpha=0.7, label='Peak Spike Activity')

            # Set y-axis labels for raster plot
            display_ticks = []
            display_labels = []
            tick_interval = 100
            for i in range(0, total_neurons, tick_interval):
                display_ticks.append(i)
                display_labels.append(str(i))
            if total_neurons > 0 and (total_neurons - 1) % tick_interval != 0:
                if (total_neurons - 1) not in display_ticks: 
                    display_ticks.append(total_neurons - 1)
                    display_labels.append(str(total_neurons - 1))
                combined_sorted = sorted(zip(display_ticks, display_labels))
                display_ticks = [t for t, l in combined_sorted]
                display_labels = [l for t, l in combined_sorted]

            ax_raster.set_yticks(display_ticks, display_labels)
            ax_raster.set_ylim(-0.5, total_neurons - 0.5) 

            # Add titles and labels for raster plot
            ax_raster.set_title('Neuron Spiking Raster Plot with Peak Activity Markers')
            ax_raster.set_ylabel('Neuron ID') 
            ax_raster.grid(True, axis='x', linestyle='--', alpha=0.7)
            ax_raster.legend(loc='best')


        # --- Average Neuron Voltage Plot Subplot ---
        ax_voltage = axes[1]
        
        # Calculate the average voltage across all neurons for each time step
        # .mean(axis=1) computes the mean for each row (each time step)
        average_voltages = voltage_columns_df.mean(axis=1)

        ax_voltage.plot(time_column, average_voltages, color='blue', linewidth=1)
        ax_voltage.set_title('Average Voltage Trace of All Neurons')
        ax_voltage.set_xlabel('Time (ms)')
        ax_voltage.set_ylabel('Average Voltage (mV)')
        ax_voltage.grid(True)
        ax_voltage.set_xlim(ax_raster.get_xlim()) # Match x-axis limits with raster plot


        plt.tight_layout() # Adjust layout to prevent labels from overlapping
        plt.show()

    except Exception as e:
        print(f"An error occurred while processing the CSV file: {e}")
        print("Please ensure the CSV file is correctly formatted with numerical data and no unexpected characters.")

if __name__ == "__main__":
    plot_neuron_voltages("output.csv", spike_threshold=0.0)

