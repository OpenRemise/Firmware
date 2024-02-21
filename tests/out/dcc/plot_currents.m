function plot_currents(name, decoder)

# Plot NAKs
figure();
plot(decoder.nak{1});
title([name " NAKs"]);
hold
plot(decoder.nak{2});
plot(decoder.nak{3});
plot(decoder.nak{4});

# Plot ACKs
figure();
plot(decoder.ack{1});
title([name " ACKs"]);
hold
plot(decoder.ack{2});
plot(decoder.ack{3});
plot(decoder.ack{4});

endfunction
