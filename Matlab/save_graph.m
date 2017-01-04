function [ output_args ] = save_graph(G, x)
% SAVE_GRAPH Saves the directed graph created by the surfer function.
%       G = A n-by-n sparse array with G(i, j) = 1 if node j is linked to
%       node i.
%       x = The PageRank power vector.

numNodes = size(G, 1);
outputName = ['amazon' sprintf('%d', numNodes)];

[rows, cols] = find(G);

outputFormat = '%d\t%d\n';

output = [cols - 1, rows - 1];
% output = [rows - 1, cols - 1];

[values, order] = sort(output(:,1));
output = output(order,:);

fileID = fopen([outputName '.txt'], 'w');
fprintf(fileID, '# Nodes: %d Edges: %d\n', numNodes , length(rows));
fprintf(fileID, '# FromNodeId   ToNodeId\n');

% fprintf(fileID, outputFormat, output);
for i=1:length(cols)
    fprintf(fileID, outputFormat, output(i, :));
end

fclose(fileID);

fileID = fopen([outputName '_prob.txt'], 'w');
for i=1:length(x)
    fprintf(fileID, '%f\n', x(i));
end

fclose(fileID);

end

