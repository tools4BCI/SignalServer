
c_files = ' ';
c_files = [c_files ' ../../src/tia/data_packet.cpp ../../src/tia/datapacket/raw_mem.cpp'];
c_files = [c_files ' ../../src/tia/constants.cpp ../../src/tia/config/control_message_decoder.cpp'];
c_files = [c_files ' ../../src/tia/config/control_messages.cpp ../../src/tia/config/control_message_encoder.cpp'];
c_files = [c_files ' ../../src/tia/tia_client.cpp ../../src/tia/client/tia_client_impl.cpp'];

unix_build_command = 'mex -v -O -DTIXML_USE_TICPP  -I../../include/  -I../../extern/include/  -lboost_thread -lboost_system -lstdc++ -outdir build';

win_boost_path = 'C:\Programme\boost\boost_1_44';

win_build_command  = ['mex -v -O -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 -I' win_boost_path ' '];
win_build_command  = [ win_build_command ' -I../../include/  -I../../extern/include/ -L' win_boost_path '\lib -outdir build'];
win_libs = ' -llibboost_system-vc100-mt-1_44 -llibboost_date_time-vc100-mt-1_44 -llibboost_regex-vc100-mt-1_44 -llibboost_thread-vc100-mt-1_44';

ticpp_files = ' ../../extern/include/ticpp/tinyxmlparser.cpp ../../extern/include/ticpp/tinyxmlerror.cpp';
ticpp_files = [ticpp_files ' ../../extern/include/ticpp/tinystr.cpp ../../extern/include/ticpp/tinyxml.cpp ../../extern/include/ticpp/ticpp.cpp'];

if(~exist('build','dir'))
  mkdir('build');
end


if(isunix)
  if(strcmp(computer, 'GLNX86'))
    eval([unix_build_command  ' src/mex_get_config.cpp ' ...
      c_files ' ../../extern/lib/ticpp/linux/libticpp.a']);

    eval([unix_build_command  ' src/simulink_get_data.cpp ' ...
      c_files ' ../../extern/lib/ticpp/linux/libticpp.a']);
  else
    eval([unix_build_command  ' src/mex_get_config.cpp ' ... 
      c_files ' ../../extern/lib/ticpp/linux/libticpp_64.a']);
        
    eval([unix_build_command  ' src/simulink_get_data.cpp ' ...
      c_files ' ../../extern/lib/ticpp/linux/libticpp_64.a']);
  end

else
  if(~exist(win_boost_path,'dir'))    error('Boost not found!');  end
  
  eval([win_build_command  ' src/mex_get_config.cpp ' ...
    c_files ticpp_files ' ../../extern/lib/ticpp/linux/libticpp.a' win_libs]);
  
  eval([win_build_command  ' src/simulink_get_data.cpp ' ...
    c_files ticpp_files ' ../../extern/lib/ticpp/linux/libticpp.a' win_libs]);
  
end


% mex -v -O -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 ...
%   -IC:\Programme\boost\boost_1_44  -I../../include  -I../../extern/include/  ...
%   -LC:\Programme\boost\boost_1_44\lib  ...
%   -outdir build ...
%     src/simulink_get_data.cpp ...
%     ../../src/datapacket/data_packet.cpp ../../src/datapacket/raw_mem.cpp ...
%     ../../src/definitions/constants.cpp ../../src/config/control_message_decoder.cpp ...
%     ../../src/config/control_messages.cpp ../../src/config/control_message_encoder.cpp ...
%     ../../src/signalserver-client/ssclient.cpp ../../src/signalserver-client/ssclientimpl.cpp ...
%     ../../extern/include/ticpp/tinyxmlparser.cpp ../../extern/include/ticpp/tinyxmlerror.cpp ...
%     ../../extern/include/ticpp/tinystr.cpp ../../extern/include/ticpp/tinyxml.cpp ../../extern/include/ticpp/ticpp.cpp ...
%     ../../extern/include/LptTools/LptTools.cpp ...
%     -llibboost_system-vc100-mt-1_44 -llibboost_date_time-vc100-mt-1_44 ...
%     -llibboost_regex-vc100-mt-1_44 -llibboost_thread-vc100-mt-1_44
