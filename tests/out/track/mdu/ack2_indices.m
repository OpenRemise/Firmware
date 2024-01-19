function [s, e] = ack2_indices(ch, ackreq, sr)
  s = ceil(length(ch) - 4 * ackreq / sr);
  e = ceil(length(ch) - 1 * ackreq / sr);
endfunction
