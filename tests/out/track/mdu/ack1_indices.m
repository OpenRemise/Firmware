function [s, e] = ack1_indices(ch, ackreq, sr)
  s = ceil(length(ch) - 8 * ackreq / sr);
  e = ceil(length(ch) - 5 * ackreq / sr);
endfunction
