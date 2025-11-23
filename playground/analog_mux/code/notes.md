at 200us between reads the ADC output looked stable, reading 1.94 +- 0.01
at this polling frequency I tested the waveform with a scope, and found the rise time on the output to be 4.72us. The fall time was even less, 1.36us. The ADC read itself takes around 2us, so we should be safe with a delay of about 5 us.

checking with scope from start of input data, I get a delay of 5.64us. ADC delay is neglibible compared to mux, so for reading I will choose 6 us.