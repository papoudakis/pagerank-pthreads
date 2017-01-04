clear all
close all

[G1,V1, E1] = loadGraph('web-Google.txt');
D1 = 2*E1/(V1*(V1-1))

[G2,V2, E2] = loadGraph('web-NotreDame.txt');
D2 = 2*E2/(V2*(V2-1))


[G3,V3, E3] = loadGraph('web-Stanford.txt');
D3 = 2*E3/(V3*(V3-1))

[G4,V4, E4] = loadGraph('web-BerkStan.txt');
D4 = 2*E4/(V4*(V4-1))

D =[D1 D2 D3 D4; 0 0 0 0];

h1 = figure(1);
bar(D)


title(r'\fontsize{16} Graph Density');
ylabel('Density')
xlabel('Graph');
legend('web-Google' , 'web-NotreDame' , 'web-Stanford', 'webBerkStan' );
xlim([0.6 1.4])

saveas(h1, 'graphDensity', 'png')
