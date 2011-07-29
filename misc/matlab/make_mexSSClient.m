
if(0)
if(~isunix)
  % This mex file is for Visual Studio 2005
  % If you have an error on startup of the client somthing like "??? Invalid
  % MEX-file" read the following:
  % One version of the boost library needs the Microsoft Visual C++ 2005 
  % Service Pack 1 Redistributable Package ATL Security Update installed. 
  % You can find it under the link
  % http://www.microsoft.com/downloads/details.aspx?familyid=766A6AF7-EC73-40FF-B072-9112BAB119C2&displaylang=en
 
  mex -v -O -outdir build -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 ...
    -IC:\Programme\boost\boost_1_46_1 -I../../include -I../../extern/include ...
    -LC:\Programme\boost\boost_1_46_1\lib ../../lib/tia.lib src/mexSSClient.cpp ...
    ../../src/ssclient_main.cpp -llibboost_system-vc100-mt-1_46_1 -llibboost_date_time-vc100-mt-1_46_1 ...
    -llibboost_random-vc100-mt-1_46_1 -llibboost_thread-vc100-mt-1_46_1 -llibboost_regex-vc100-mt-1_46_1
  %   mex -v -O -outdir build -DTIXML_USE_TICPP -DWIN32
  %   -D_WIN32_WINNT=0x0501 -IC:\Programme\boost\boost_1_46_1 -I../../include -I../../extern/include -LC:\Programme\boost\boost_1_46_1\lib -L../../extern/lib/ticpp/win src/mexSSClient.cpp  ../../src/datapacket/data_packet.cpp ../../src/datapacket/raw_mem.cpp ../../src/definitions/constants.cpp ../../src/signalserver-client/ssclientimpl.cpp ../../src/config/control_message_encoder.cpp ../../src/config/control_message_decoder.cpp ../../src/signalserver-client/ssclient.cpp ../../src/config/control_messages.cpp   ../../extern/include/ticpp/tinyxmlparser.cpp ../../extern/include/ticpp/tinyxmlerror.cpp ../../extern/include/ticpp/tinystr.cpp ../../extern/include/ticpp/tinyxml.cpp ../../extern/include/ticpp/ticpp.cpp -llibboost_system-vc80-mt-1_42 -llibboost_date_time-vc80-mt-1_42 -llibboost_regex-vc80-mt-1_42 -llibboost_thread-vc80-mt-1_42
else 
  mex -cxx -O -DWall -DTIXML_USE_TICPP -lboost_thread -lboost_system mexSSClient.cpp    ../data_packet.o ../raw_mem.o ../constants.o ../ssclient/ssclientimpl.o ../control_message_encoder.o ../control_message_decoder.o ../ssclient/ssclient.o ../control_messages.o ../lib_ticpp/libticpp.a   
end

end

%  c_files = ' ';
%  c_files = [c_files ' ../../src/tia/data_packet.cpp ../../src/tia/datapacket/raw_mem.cpp'];
%  c_files = [c_files ' ../../src/tia/data_packet3.cpp ../../src/tia/datapacket/raw_mem3.cpp ../../src/tia/clock.cpp'];
%  c_files = [c_files ' ../../src/tia/constants.cpp ../../src/tia/config/control_message_decoder.cpp'];
%  c_files = [c_files ' ../../src/tia/config/control_messages.cpp ../../src/tia/config/control_message_encoder.cpp'];
%  c_files = [c_files ' ../../src/tia/tia_client.cpp ../../src/tia/client/tia_client_impl.cpp'];
%  c_files = [c_files ' ../../src/tia/client/tia_new_client_impl.cpp'];
%  c_files = [c_files ' ../../src/tia/newtia/network_impl/boost_tcp_socket_impl.cpp ../../src/tia/newtia/network_impl/boost_udp_read_socket.cpp'];
%  c_files = [c_files ' ../../src/tia/newtia/tia_datapacket_parser.cpp ../../src/tia/newtia/tia_meta_info_parse_and_build_functions.cpp'];
%  c_files = [c_files ' ../../src/tia/newtia/messages_impl/tia_control_message_parser_1_0.cpp'];
%  
%  
%  unix_build_command = 'mex -v -O -DTIXML_USE_TICPP  -I../../ -I../../include/  -I../../extern/include/  -lboost_thread -lboost_system -lstdc++ -outdir build';
%  
%  win_boost_path = 'C:\Programme\boost\boost_1_44';
%  
%  win_build_command  = ['mex -v -O -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 -I' win_boost_path ' '];
%  win_build_command  = [ win_build_command ' -I../../include/  -I../../extern/include/ -L' win_boost_path '\lib -outdir build'];
%  win_libs = ' -llibboost_system-vc100-mt-1_44 -llibboost_date_time-vc100-mt-1_44 -llibboost_regex-vc100-mt-1_44 -llibboost_thread-vc100-mt-1_44';
%  
%  ticpp_files = ' ../../extern/include/ticpp/tinyxmlparser.cpp ../../extern/include/ticpp/tinyxmlerror.cpp';
%  ticpp_files = [ticpp_files ' ../../extern/include/ticpp/tinystr.cpp ../../extern/include/ticpp/tinyxml.cpp ../../extern/include/ticpp/ticpp.cpp'];
%  
%  if(~exist('build','dir'))
%    mkdir('build');
%  end
%  
%  
%  if(isunix)
%    if(strcmp(computer, 'GLNX86'))
%      eval([unix_build_command  ' src/mexSSClient.cpp ' ...
%        c_files ' ../../extern/lib/ticpp/linux/libticpp.a']);
%  
%      eval([unix_build_command  ' src/simulink_get_data.cpp ' ...
%        c_files ' ../../extern/lib/ticpp/linux/libticpp.a']);
%    else
%      eval([unix_build_command  ' src/mex_get_config.cpp ' ...
%        c_files ' ../../extern/lib/ticpp/linux/libticpp_64.a']);
%  
%      eval([unix_build_command  ' src/simulink_get_data.cpp ' ...
%        c_files ' ../../extern/lib/ticpp/linux/libticpp_64.a']);
%    end
%  
%  else
%    if(~exist(win_boost_path,'dir'))    error('Boost not found!');  end
%  
%    eval([win_build_command  ' src/mex_get_config.cpp ' ...
%      c_files ticpp_files ' ../../extern/lib/ticpp/linux/libticpp.a' win_libs]);
%  
%    eval([win_build_command  ' src/simulink_get_data.cpp ' ...
%      c_files ticpp_files ' ../../extern/lib/ticpp/linux/libticpp.a' win_libs]);
%  
%  end

