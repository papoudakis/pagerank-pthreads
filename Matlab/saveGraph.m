function saveGraph(X , filename)
fid=fopen(filename, 'w');
fprintf(fid, ' %f\n' , X');
fclose(fid);
