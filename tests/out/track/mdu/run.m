clear
clc
close all

load mn180

fs = 83333;
channels = 2;
sr = 10^6 / (fs / channels);
[s, e] = ack1_indices(tf0_ch1, 3600, sr)

##plot_currents("mn180")
##plot_currents("ms450")
##plot_currents("ms491")
##plot_currents("ms990")

##plot_currents_reversed("mn180")
##plot_currents_reversed("ms450")
##plot_currents_reversed("ms491")
##plot_currents_reversed("ms990")
