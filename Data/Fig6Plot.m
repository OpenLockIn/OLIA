set(0, 'defaultAxesFontSize',20);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesLineWidth', 2);

figure('outerposition',[0 0 1000 1200]);
set(gcf,'color','w');
pause(0.5);
axes('Position',[0.15 0.3 0.8 0.65]);

plot(setPhase, measuredPhase, 'o','color','k');
hold

p = plot(fitxPlusb);
set(p, 'color', 'k');
legend("hide");

%xlabel("Set phase (deg)");
xlabel("");
ylabel("Measured phase, {\fontname{Cambria Math}{\it\phi}} (deg)");
text(-1, 89,'(a)','fontsize',20);

set(gca,'xLim',[-2, 92]);
set(gca,'yLim',[-2, 92]);

set(gca,'xTick',[0, 10, 20, 30, 40, 50, 60, 70, 80, 90]);
set(gca,'xTickLabel',[]);
%set(gca,'xTickLabel',[0, 10, 20, 30, 40, 50, 60, 70, 80, 90]);

set(gca,'yTick',[0, 10, 20, 30, 40, 50, 60, 70, 80, 90]);
set(gca,'yTickLabel',[0, 10, 20, 30, 40, 50, 60, 70, 80, 90]);

axes('Position',[0.15 0.1 0.8 0.18]);
box on
set(gca,'xTick',[0, 10, 20, 30, 40, 50, 60, 70, 80, 90]);
set(gca,'xTickLabel',[0, 10, 20, 30, 40, 50, 60, 70, 80, 90]);

RVolts = R*(3.3/2047);

plot(setPhase, RVolts,'o','color','k');
set(gca,'xLim',[-2, 92]);

set(gca,'yLim',[0.236, 0.240]);
set(gca,'yTick',[0.236 0.238 0.240]);
set(gca,'yTickLabel',[236 238 240]);
yline(mean(RVolts),'linewidth',2,'color','k');
xlabel("Set phase, {\fontname{Cambria Math}{\it\phi_s - \phi_r}}  (deg)");
ylabel("{\fontname{Cambria Math}{\itR}} (mV)");
text(-1, 0.2395,'(b)','fontsize',20);