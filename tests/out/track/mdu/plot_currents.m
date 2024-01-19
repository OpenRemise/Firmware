function plot_currents(workspace)
  s = load(workspace);
  impl([workspace " tf0"], s.tf0_ch1, s.tf0_ch2, s.tf0_ch12, 3600);
  impl([workspace " tf1"], s.tf1_ch1, s.tf1_ch2, s.tf1_ch12, 60);
  impl([workspace " tf2"], s.tf2_ch1, s.tf2_ch2, s.tf2_ch12, 60);
  impl([workspace " tf3"], s.tf3_ch1, s.tf3_ch2, s.tf3_ch12, 120);
  impl([workspace " tf4"], s.tf4_ch1, s.tf4_ch2, s.tf4_ch12, 225);
endfunction

function impl(str, ch1, ch2, ch12, ackreq)
  fs = 83333;
  channels = 2;
  sr = 10^6 / (fs / channels);

  # Make measurements even
  shortest = min([length(ch1) length(ch2) length(ch12)]);
  ch1 = ch1(length(ch1)-shortest+1:end);
  ch2 = ch2(length(ch2)-shortest+1:end);
  ch12 = ch12(length(ch12)-shortest+1:end);

  figure;
  hold
  plot(ch1)
  title([str " ch1 | ch2"]);
  [s, e] = ack1_indices(ch1, ackreq, sr);
  xline(s, "linestyle", "--", "color", "m");
  xline(e, "linestyle", "--", "color", "m");
  plot(ch2)
  [s, e] = ack2_indices(ch2, ackreq, sr);
  xline(s, "linestyle", "--", "color", "c");
  xline(e, "linestyle", "--", "color", "c");

  figure;
  hold
  plot(ch12)
  title([str " ch1 & ch2"]);
  [s, e] = ack1_indices(ch12, ackreq, sr);
  xline(s, "linestyle", "--", "color", "m");
  xline(e, "linestyle", "--", "color", "m");
  [s, e] = ack2_indices(ch12, ackreq, sr);
  xline(s, "linestyle", "--", "color", "c");
  xline(e, "linestyle", "--", "color", "c");
endfunction
