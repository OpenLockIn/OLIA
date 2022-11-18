figure('outerposition',[0 0 1500 750]);
set(gcf,'color','w');
pause(0.5);

axes('Position',[0.08 0.15 0.4 0.8]);
hold on
box on

extracted_sigVolts = extracted_sig*(3.3/2047);
extracted_sigmVolts = extracted_sigVolts*1000;

plot(snr, extracted_sigmVolts,'o', 'linewidth',2,'linestyle','none','color','k')


xlabel('Signal to noise ratio, {\fontname{Cambria Math}{\gamma}}');
ylabel('Measured amplitude, {\fontname{Cambria Math}{\itR}} (mV)');

set(gca, 'XScale', 'log')
set(gca,'xLim',[0.0001, 1000]);
set(gca,'yLim',[0, 2]);

text(0.00012,1.92,'(a)','fontsize',20);
yline(true,'-','linewidth',2);

axes('Position',[0.58 0.15 0.4 0.8]);
hold on
box on

inPhaseV = inPhase*(3.3/2047);
inPhasemV = inPhaseV*1000;
plot(harmonicNumber, inPhasemV, 'o','color','k');
set(gca,'xLim',[1, 23]);

hold on


p = plot(oddFitmV);
set(p, 'color', 'k');

set(gca,'xLim',[-1, 23]);
set(gca,'yLim',[-5, 280]);
legend("hide");
xlabel("Harmonic number,{\fontname{Cambria Math}{\it k}} ");
ylabel("Measured amplitude, {\fontname{Cambria Math}{\itR}}  (mV)");
text(-0.75,265,'(b)','fontsize',20);