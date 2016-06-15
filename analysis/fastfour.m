% This function outputs frequency vs. amplitude plot of an input given time series
% The function requires the following inputs
% signal=times series data
% sam=frequency of sampling
%

function [fouranal]=fastfour(signal,sam);
Sample_rate=sam;
n=(length(signal));
fftsignal=fft(signal,n);

%abs_fourier_transform=abs(fftsignal)./(n/2);
abs_fourier_transform=abs(fftsignal)./(length(fftsignal)/2);

frequency=0:sam/n:sam-1/n;
fouranal=[frequency' abs_fourier_transform];