We want to read a shift register by giving it a clock signal. The register loads data when shift/load (SH/LD) is low. When we set it high and introduce a clock signal, the registers should start shifting the data.

Try pull down resistors at first, so input is off default

*notes on PIO*
The statemachine stalls when RX is full on default assembly, pulling has the same logic btw
Maybe using  mov rxfifo[y], isr could be beneficial for us, we block the RX FIFO and at the same time we put in the data, alternatively we could read all for words from the RX FIFO

*notes on shift register*
Instead of setting clock inhib to high, we can set clock to constant high directly, since they are connected by an OR gate

### test 1 - check with scope

1) set up the circuit with one button, scope and pulldown resistor
2) write PIO code to provide clock signal and LD signal, provide a single pulse on the latch and then keep pulsing the clock
3) check shift register output on scope

**TEST PASSED SUCCESFULLY**
 * make sure to ground all input, especially SERIAL properly
 * shift register stable at clock divider 4, at 3 issues start appearing, fails at 2, will run at like 20 or 10 probably
 * time difference between clock signal and returning signal about 34 ns

 ### test 2 - read serial in data

1) same setup as test 1, but pin 13 will read data
2) add an input pin to PIO loop
3) clear fifo, allow to fill up, read

**TEST PASSED SUCCESFULLY**
 * was able to rad a different value by changing the button press
 * some weird timings of read operations noted on scope

now need to decode the signal received and figure out fifo timings
so I noticed that with the naive implementation, where the state machine just stalls, the first thing that gets dumped into the RX FIFO is an old value, from the previous measurement cycle, need to clear it out somehow

one fix would be to add an empty push, after reading ALL the buttons, so for 128 registers, we would put this push after 4 normal pushes, this of course brings extensibility issues, but should be stable
Another fix might be to use pio_sm_restart() - ok so this wont work, since it wont reset the PC, but calling init might, or writing an explicit jump instruction
If we are reading 32 buttons, then just discarding the first word we get is perfectly fine. - implement this first