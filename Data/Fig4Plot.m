set(0, 'defaultAxesFontSize',20);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesLineWidth', 2);

figure('outerposition',[0 0 1500 750]);
set(gcf,'color','w');
pause(0.5);

axes('Position',[0.08 0.15 0.4 0.8]);
hold on
box on

text(971, 0.24,'(a)','fontsize',20);

%colour1 = [0, 0.4470, 0.7410];
%colour2 = [0.8500, 0.3250, 0.0980];
%olour3 = [0, 0.482, 0.537];

colour1 = 'k';
colour2 = 'k';
colour3 = 'k';

R6Volts = R6*(3.3/2047);
R06Volts = R06*(3.3/2047);
R006Volts = R006*(3.3/2047);

plot(f6,R6Volts,'x', 'color',colour1);
plot(f06,R06Volts,'x', 'color',colour2);
plot(f006,R006Volts,'x', 'color',colour3);


p6 = plot(response6Volts);
%p6 = plot(response6, f6, r6);
set(p6, 'color',colour1); 
%set(p6, 'color','k');

%plot(f06,r06,'x');
p06 = plot(response06Volts);
set(p06, 'color',colour2); 
%set(p06, 'color','k');
set(p06, 'linestyle','--'); 

%plot(f006,r006,'x');
p006 = plot(response006Volts);
%set(p006, 'color',[0.9290, 0.6940, 0.1250]); 
set(p006, 'color',colour3);
set(p006, 'linestyle',':'); 

set(gca, 'xlim', [970 1030]);
set(gca, 'ylim', [0 0.25]);
xlabel('Input frequency,{\fontname{Cambria Math}{\it f}_s} (Hz)');
ylabel('Amplitude,{\fontname{Cambria Math}{\it R}} (V)');
hold off

hl = legend;
hl.String{1} = '6 s';
hl.String{2} = '0.6 s';
hl.String{3} = '0.06 s';
%legend("hide");
legend boxoff

axes('Position',[0.58 0.15 0.4 0.8]);
hold on
box on
text(-4, 0.385,'(b)','fontsize',20);

xlim([-5 60]);
%ylim([0 250]);

xlabel("Time (s)");
ylabel("Amplitude,{\fontname{Cambria Math}{\it R}} (V)");

r6Volts = r6*(3.3/2047);
r06Volts = r06*(3.3/2047);
r006Volts = r006*(3.3/2047);


plot(timeZeroed,r6Volts, 'color',colour1)
plot(timeZeroed,r06Volts, 'linestyle','--', 'color',colour2)
plot(timeZeroed,r006Volts, 'linestyle',':', 'color',colour3)

legend("6 s", "0.6 s", "0.06 s",'Location','southeast')
legend boxoff

% axes('Position',[0.13, 0.7, 0.12, 0.2]);
% plot(timeConstantSet, width, 'o','color','k')
% set(gca, 'XScale', 'log')
% set(gca, 'Xtick', [0.06 0.6 6])
% set(gca, 'XLim', [0.05 7])
% set(gca, 'yLim', [-1 25])
% ylabel('\Deltaf _1_% (Hz)','fontsize',14);
% xlabel('\tau (s)','fontsize',14);
