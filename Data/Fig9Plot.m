set(0, 'defaultAxesFontSize',20);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesLineWidth', 2);



figure('outerposition',[0 0 1000 1000]);
set(gcf,'color','w');
pause(0.5);

axes('Position',[0.1 0.1 0.8 0.78]);

r_extVolts = r_ext*(3.3/2047);
r_intVolts = r_int*(3.3/2047);

plot(int_amps, r_intVolts,'.','MarkerSize', 30,'Color','k');

hold on

plot(ext_amps, r_extVolts,'x','MarkerSize',15,'Color','k');


set(gca, 'XScale', 'log')
set(gca, 'YScale', 'log')

set(gca, 'XLim', [1E-11 1E-5])

set(gca, 'Xtick', [1E-10 1E-9 1E-8 1E-7 1E-6])
set(gca, 'XtickLabel', [1 10 100 1000 10000])

xlabel("Intensity, \iota (a.u.)");
ylabel("Measured amplitude,{\fontname{Cambria Math}{{\it R}}} (V)");

legend({'Internal referencing','External referencing'},'location','northwest')
legend boxoff

ax1 = gca; % current axes
ax1.XColor = 'k';
ax1.YColor = 'k';
ax1_pos = ax1.Position; % position of first axes

ax2 = axes('Position',ax1_pos,...
'XAxisLocation','top',...
'YAxisLocation','right',...
'Color','none');
ax2.XColor = 'k';

set(gca, 'XScale', 'log')
set(gca, 'YTick', [])

set(gca, 'XLim', [1E-11 1E-5])
xlabel("Measured photocurrent,{\fontname{Cambria Math}{{\it I}_{ph}}} (A)");
