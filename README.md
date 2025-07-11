# ratbrain
An implementation of the Izhikevich Model of a neuron for SNNs.

GENERATING DATA

Use the following command to compile ratbrain.c into an executable:

gcc main.c ratbrain.c -o rat


VISUALIZING DATA

The main.c program I wrote is a simple loop that will create a brain with 100 neurons, track its activity for 1000 ms, destroy and brain, and then repeat forever. I used a simple graphical representaiton that prints pipes for spikes creating a vertical scrolling rastor plot. Not very useful other than to visualize the spiking activity.
