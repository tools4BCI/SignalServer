
clear all;

fs = 128;

fs2 = 512.1;

dur = 12 * 60 * 60;    % 12h - 60min - 60s

t = 1 : 1/fs2 :dur  -1/fs2;
len = length(t);


  div = gcd(fs,round(fs2))
  div = div + fs2 -round(fs2);

loss = abs(fs2/(fs-div))

if loss ~= Inf
  n = 1: loss :len;
  s(int64(n)) = [];
end


s = sin(25 * 2*pi*t);
N = 2^nextpow2(length(s));
f = 2* abs(fft(s,N)/len);

figure;
plot((0:1:N/2-1)/(N/fs),f(1:N/2));
xlim([15 35]);
ylim([-0.05 0.6]);
title(['Sampling Rates: ' num2str(fs) ' and ' num2str(fs2)]);
xlabel(['f [Hz]']);
ylabel(['normalized Amplitude']);