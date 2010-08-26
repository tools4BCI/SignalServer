
if(~isunix)
  % This mex file is for Visual Studio 2005
  % If you have an error on startup of the client somthing like "??? Invalid
  % MEX-file" read the following:
  % One version of the boost library needs the Microsoft Visual C++ 2005 
  % Service Pack 1 Redistributable Package ATL Security Update installed. 
  % You can find it under the link
  % http://www.microsoft.com/downloads/details.aspx?familyid=766A6AF7-EC73-40FF-B072-9112BAB119C2&displaylang=en
 
  mex -v -O -outdir build -DTIXML_USE_TICPP -DWIN32 -D_WIN32_WINNT=0x0501 -IC:\Programme\boost\boost_1_42 -I../../include -I../../extern/include -LC:\Programme\boost\boost_1_42\lib -L../../extern/lib/ticpp/win src/mexSSClient.cpp  ../../src/datapacket/data_packet.cpp ../../src/datapacket/raw_mem.cpp ../../src/definitions/constants.cpp ../../src/signalserver-client/ssclientimpl.cpp ../../src/config/control_message_encoder.cpp ../../src/config/control_message_decoder.cpp ../../src/signalserver-client/ssclient.cpp ../../src/config/control_messages.cpp   ../../extern/include/ticpp/tinyxmlparser.cpp ../../extern/include/ticpp/tinyxmlerror.cpp ../../extern/include/ticpp/tinystr.cpp ../../extern/include/ticpp/tinyxml.cpp ../../extern/include/ticpp/ticpp.cpp -llibboost_system-vc80-mt-1_42 -llibboost_date_time-vc80-mt-1_42 -llibboost_regex-vc80-mt-1_42 -llibboost_thread-vc80-mt-1_42
else 
  mex -cxx -O -DWall -DTIXML_USE_TICPP -lboost_thread -lboost_system mexSSClient.cpp    ../data_packet.o ../raw_mem.o ../constants.o ../ssclient/ssclientimpl.o ../control_message_encoder.o ../control_message_decoder.o ../ssclient/ssclient.o ../control_messages.o ../lib_ticpp/libticpp.a   
end