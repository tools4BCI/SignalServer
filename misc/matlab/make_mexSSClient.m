
if(~isunix)
  %   mex -v -O -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 -IC:\Programme\boost\boost_1_42 -LD:\graz\signalserver\lib_ticpp mexSSClient.cpp  ../hardware/data_packet.cpp ../hardware/raw_mem.cpp ../constants.cpp ../ssclient/ssclientimpl.cpp ../control_message_encoder.cpp ../control_message_decoder.cpp ../ssclient/ssclient.cpp ../control_messages.cpp   ../ticpp/tinyxmlparser.cpp ../ticpp/tinyxmlerror.cpp ../ticpp/tinystr.cpp ../ticpp/tinyxml.cpp ../ticpp/ticpp.cpp "C:\Programme\boost\boost_1_42\lib\libboost_system-vc80-mt-1_42.lib" "C:\Programme\boost\boost_1_42\lib\libboost_date_time-vc80-mt-1_42.lib" "C:\Programme\boost\boost_1_42\lib\libboost_regex-vc80-mt-1_42.lib" "C:\Programme\boost\boost_1_42\lib\libboost_thread-vc80-mt-1_42.lib"
  %  mex -v -O -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 -IC:\Programme\boost\boost_1_42 -L../lib_ticpp -LC:\Programme\boost\boost_1_42\lib  mexSSClient.cpp  ../hardware/data_packet.cpp ../hardware/raw_mem.cpp ../constants.cpp ../ssclient/ssclientimpl.cpp ../control_message_encoder.cpp ../control_message_decoder.cpp ../ssclient/ssclient.cpp ../control_messages.cpp   ../ticpp/tinyxmlparser.cpp ../ticpp/tinyxmlerror.cpp ../ticpp/tinystr.cpp ../ticpp/tinyxml.cpp ../ticpp/ticpp.cpp   -llibboost_system-vc80-mt-1_42 -llibboost_date_time-vc80-mt-1_42 -llibboost_regex-vc80-mt-1_42 -llibboost_thread-vc80-mt-1_42
  
  mex -v -O -outdir build -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 -IC:\Programme\boost\boost_1_42 -I../../include -I../../extern/include -LC:\Programme\boost\boost_1_42\lib -L../../extern/lib/ticpp/win src/mexSSClient.cpp  ../../src/datapacket/data_packet.cpp ../../src/datapacket/raw_mem.cpp ../../src/definitions/constants.cpp ../../src/signalserver-client/ssclientimpl.cpp ../../src/config/control_message_encoder.cpp ../../src/config/control_message_decoder.cpp ../../src/signalserver-client/ssclient.cpp ../../src/config/control_messages.cpp   ../../extern/include/ticpp/tinyxmlparser.cpp ../../extern/include/ticpp/tinyxmlerror.cpp ../../extern/include/ticpp/tinystr.cpp ../../extern/include/ticpp/tinyxml.cpp ../../extern/include/ticpp/ticpp.cpp -llibboost_system-vc80-mt-1_42 -llibboost_date_time-vc80-mt-1_42 -llibboost_regex-vc80-mt-1_42 -llibboost_thread-vc80-mt-1_42
else 
  mex -cxx -O -DWall -DTIXML_USE_TICPP -lboost_thread -lboost_system mexSSClient.cpp    ../data_packet.o ../raw_mem.o ../constants.o ../ssclient/ssclientimpl.o ../control_message_encoder.o ../control_message_decoder.o ../ssclient/ssclient.o ../control_messages.o ../lib_ticpp/libticpp.a   
end