set(0, 'defaultAxesFontSize', 20);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesLineWidth', 2);

figure('outerposition',[0 0 1000 1000]);
set(gcf,'color','w');
pause(0.5);
hold on
box on

plot(setVGain1, measuredVGain1,'o')

plot(setVGain16, measuredVGain16,'.','MarkerSize',24)

plot(setVGain64(2:20), measuredVGain64(2:20),'x','MarkerSize',10,'color',[0.4660, 0.6740, 0.1880])

yline(measuredVGain64(1),'--','linewidth',2);


set(gca, 'XScale', 'log')
set(gca, 'YScale', 'log')
set(gca, 'XLim', [1E-6 2])
set(gca, 'YLim', [5E-7 2])

xlabel("Set amplitude,{\fontname{Cambria Math}{\it S}_0} (V)");
ylabel("Measured amplitude,{\fontname{Cambria Math}{\it R}} (V)");

legend("Input gain = 1", "Input gain = 16", "Input gain = 64",'Location','northwest')
legend boxoff