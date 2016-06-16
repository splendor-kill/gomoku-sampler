生成对弈数据集
1. 配置config.txt
  a)max_time 设为-1时，迭代次数将为iterations_param1 和 iterations_param2 之间的一个随机值
  b)对每一个迭代次数，会重复下number_of_repeat 次
  c)verbose 控制输出到控制台的信息量
  d)生成的数据文件放在dat_dir 下，需要保证有此目录和权限，程序不自动处理这个问题
  e)数据文件名为 <file_prefix>+时间 的形式，每个文件大小约 <file_size(K)>
2. 按q键结束
