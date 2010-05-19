function  test_mexSSClient()
  [master_info sig_info ch_info] = mexSSClient('localhost',9000,'tcp')
  
  for i = 1:100
    data = mexSSClient()
  end
  
  mexSSClient('close');
end