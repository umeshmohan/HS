function PlotHeadandThorax(data)
    Head_1 = [data(:, 1) data(:, 2)]; % specifying coordinates
    Head_2 = [data(:, 3) data(:, 4)];
    Thorax_1 = [data(:, 5) data(:, 6)];
    Thorax_2 = [data(:, 7) data(:, 8)];
    Head = Head_2 - Head_1;  % specifying lines
    Thorax = Thorax_2 - Thorax_1;
    theta1 = (atan2(Head(:, 2), Head(:, 1))); % datangle between Head and X-axis
    theta2 = (atan2(Thorax(:, 2), Thorax(:, 1))); % datangle between Thorax and X-axis
    figure;
    plot(theta1, 'r'); hold on;
    plot(theta2, 'g');
    thetahead = 180 * (unwrap(theta1)) / pi; % to make angles continous
    thetathorax = 180 * (unwrap(theta2)) / pi;
    theta = thetahead - thetathorax; % angle between head and thorax
    Z = length(data) / 3000;
    X = 0:Z / (length(data) - 1):Z; % Converting X-axis to time
    figure;
    plot(X, thetathorax, 'k');
    hold on;
    plot(X, thetahead, 'r');
    hold on;
    plot(X, theta);
end
