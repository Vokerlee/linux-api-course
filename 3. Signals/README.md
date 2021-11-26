# File transfer via signals

This program consists of 2 different parts: receiver and transmitter.
The main aim is to transfer file from transmitter to receiver through signals handling.

## Istallation and launch

Just print the following after your `git clone`:

```
cd 3.\ Signals/
mkdir build
cd build
cmake ..
```

## How to use

Receiver begins to work (`./receiver output_file`, where `output_file` is a file name to write transmitted info) and waits for some signals (doing nothing else). 
Then transmitters (`./transmitter input_file receiver_pid`, where input_file is a file name whith info to transmit and `receiver_pid` is a receiver's process id). So transmitter transfer file info through fifo to receiver and the receiver prints this info to `output_file`.

The velocity of transmit is about `60 Mb/s`.