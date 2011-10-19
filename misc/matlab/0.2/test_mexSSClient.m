function  test_mexSSClient()
  [master_info sig_info ch_info] = TiA_matlab_client('localhost',9000,'tcp','0.2')
  
  for i = 1:100
    data = TiA_matlab_client()
  end
  
  TiA_matlab_client('close');
end