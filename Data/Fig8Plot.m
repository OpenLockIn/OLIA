set(0, 'defaultAxesFontSize',20);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesLineWidth', 2);

figure('outerposition',[0 0 1000 1000]);
set(gcf,'color','w');
pause(0.5);

plot(f_int, rNorm_int,'color','k')
%errorbar(f_int, rNorm_int, rsd_int)
hold on

plot(f_ext, rNorm_ext,'--','color','k')
%errorbar(f_ext, rNorm_ext, rsd_ext)

xlabel('Modulation frequency,{\fontname{Cambria Math}{\it f_s}} (Hz)');
ylabel('Normalised amplitude,{\fontname{Cambria Math}{{\it R/R}_{1 kHz}}} ');

set(gca, 'XScale', 'log')
set(gca, 'xlim', [10 100000])
set(gca, 'ylim', [0.7 1.05])

legend({'Internal','External'},'location','northeast')
legend boxoff