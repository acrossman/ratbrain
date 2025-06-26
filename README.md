# ratbrain
An implementation of the Izhikevich Model of a neuron for SNNs.

GENERATING DATA

Use the following command to compile ratbrain.c into an executable:

gcc ratbrain.c -o rat -lm

You can run the program with optional paramters defined by:

./rat NE NI time

examples:

./rat <- uses defaults values of 800, 200, 1000
./rat 10 <- uses value of 10 for NE
./rat 800 200 10000 <- uses values of NE = 800, NI = 200, time = 10000

note that you can use the linux function time() to track how long the program takes to execute:

time ./rat

VISUALIZING DATA

graph.py is Gemini AI generated code to graph a rastor plot with usage:

python3 graph.py
