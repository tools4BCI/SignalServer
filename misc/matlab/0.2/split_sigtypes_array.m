% [master_info channel_info naming_string] = mex_get_config_v2('127.0.0.1', 9000);

[a b c d] = mex_get_config('127.0.0.1', 9000, 'tcp');

tic

max_types = 0;

info = [];

for n = 1:size(b,1)
  if(length(b{n,2}) > max_types)
    max_types = length(b{n,2});
  end

  info = [info; b{n,1} b{n,3} b{n,4} b{n,5} ];
  
end

types = [];

for n = 1:size(b,1)
  str = [];
  if(length(b{n,2}) < max_types)
    d = max_types - length(b{n,2});
    for m = 1:d
      str = [ str 'x'];
    end
  end  
  types = [types; [b{n,2} str] ];
end



toc




