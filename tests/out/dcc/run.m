clc
clear all
close all

mn330_service_ack
ms450_service_ack
ms990_service_ack
mx645_service_ack

##plot_currents("MN330", mn330);
##plot_currents("MS990", ms990);
##plot_currents("MS450", ms450);
##plot_currents("MX645", mx645);

wlen = 5e-3 * 10000 / 2;
plot_movsums("MN330", mn330, wlen);
plot_movsums("MS990", ms990, wlen);
plot_movsums("MS450", ms450, wlen);
plot_movsums("MX645", mx645, wlen);
