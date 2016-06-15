function [frequency_array' signal_fft_abs] = fastfour(signal, sample_rate);
%Returns a fft of input signal
%
    n = (length(signal));
    signal_fft = fft(signal, n);
    signal_fft_abs = abs(signal_fft) ./ (length(signal_fft) / 2);
    frequency_array = 0:sam/n:sam-1/n;
end