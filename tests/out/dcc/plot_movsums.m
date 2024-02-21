function plot_movsums(name, decoder, wlen)

# Plot NAKs
figure();
plot(movsum(decoder.nak{1}, wlen));
title([name " NAKs movsums"]);
hold
plot(movsum(decoder.nak{2}, wlen));
plot(movsum(decoder.nak{3}, wlen));
plot(movsum(decoder.nak{4}, wlen));

# Plot ACKs
figure();
plot(movsum(decoder.ack{1}, wlen));
title([name " ACKs movsums"]);
hold
plot(movsum(decoder.ack{2}, wlen));
plot(movsum(decoder.ack{3}, wlen));
plot(movsum(decoder.ack{4}, wlen));

endfunction
