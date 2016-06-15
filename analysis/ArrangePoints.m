function sorted_data = ArrangePoints(filename)
    fileID = fopen(filename,'r');
    dataArray = textscan(fileID, '%f%f%f%f%f%f%f%f%f%[^\n\r]', inf, ...
                         'Delimiter', ',', 'HeaderLines', 9);
    fclose(fileID);
    data = [dataArray{1:end-1}];
    data = data(:, 2:9);
    distance = DistanceFromBottomMidPoint(data);
    [~,i] = sort(distance,2);
    i2 = [(i(:,1)*2)-1,(i(:,1)*2),(i(:,2)*2)-1,(i(:,2)*2), ...
          (i(:,3)*2)-1,(i(:,3)*2),(i(:,4)*2)-1,(i(:,4)*2)];
    sorted_data = RearrangePoints(data, i2);
end

function distance = DistanceFromBottomMidPoint(data)
    [n_frames, ~] = size(data);
    d2 = data - repmat([600,800], n_frames, 4);
    distance = [hypot(d2(:,1),d2(:,2)),hypot(d2(:,3),d2(:,4)), ...
                hypot(d2(:,5),d2(:,6)),hypot(d2(:,7),d2(:,8))];
end

function sorted_data = RearrangePoints (data, i2)
    sorted_data = zeros(size(data));
    for row_n = 1:length(data)
        row_data = data(row_n, (i2(row_n, :)));
        sorted_data(row_n, :) = row_data;
    end
end