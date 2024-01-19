function plot_currents_reversed(workspace)
  s = load(workspace);
  impl([workspace " tf0"], s.tf0_ch1, s.tf0_ch2, s.tf0_ch12, 3600);
  impl([workspace " tf1"], s.tf1_ch1, s.tf1_ch2, s.tf1_ch12, 60);
  impl([workspace " tf2"], s.tf2_ch1, s.tf2_ch2, s.tf2_ch12, 60);
  impl([workspace " tf3"], s.tf3_ch1, s.tf3_ch2, s.tf3_ch12, 120);
  impl([workspace " tf4"], s.tf4_ch1, s.tf4_ch2, s.tf4_ch12, 225);
endfunction

function impl(str, ch1, ch2, ch12, ackreq)
  # Reverse all measurements
  rch1 = flip(ch1);
  rch2 = flip(ch2);
  rch12 = flip(ch12);

  # Make measurements even
  shortest = min([length(rch1) length(rch2) length(rch12)]);
  rch1 = rch1(1:shortest);
  rch2 = rch2(1:shortest);
  rch12 = rch12(1:shortest);

  figure;
  plot(rch1)
  title([str " ch1 | ch2 reversed"]);
  hold
  plot(rch2)

  figure;
  plot(rch12)
  hold
  title([str " ch1 & ch2 reversed"]);
endfunction
