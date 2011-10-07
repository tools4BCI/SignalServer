% [master_info channel_info naming_string] = mex_get_config_v2('127.0.0.1', 9000);


[a b c d] = mex_get_config('127.0.0.1', 9000, 'tcp');

tic

max_types = 0;
max_names = 0;

for n = 1:size(c,1)
  if(length(c{n,1}) > max_names)
    max_names = length(c{n,1});
  end
  
  if(length(c{n,2}) > max_types)
    max_types = length(c{n,2});
  end
end

names = [];
types = [];

for n = 1:size(c,1)
  str = [];
  if(length(c{n,1}) < max_names)
    d = max_names - length(c{n,1});
    for m = 1:d
      str = [ str 'x'];
    end
  end  
  names = [names; [c{n,1}, str] ];
  
  str = [];
  if(length(c{n,2}) < max_types)
    d = max_types - length(c{n,2});
    for m = 1:d
      str = [ str 'x'];
    end
  end  
  types = [types; [c{n,2} str] ];
end

toc




