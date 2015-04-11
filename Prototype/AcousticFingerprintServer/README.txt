== 简介 ==

Acoustic Fingerprint Server 用于查询和录入phash，以便识别视频文件或者音频文件。

== Acoustic Fingerprint Server Configuration Instruction ==
== AFServer 配置说明 ==

编译
========================
1. 下载ZeroMQ源代码，在服务器上安装 ZeroMQ 2.1.3 （ASFserver 所用的版本）
   
   你可以从以下链接中获得ZeroMQ的源代码	
	http://www.zeromq.org/intro:get-the-software

2. 执行 sh build.sh bin/ （其中bin为要安装的目录）


启动
========================
./afserverd [options]

options: 

-p <portnumber>         要使用的端口号  - e.g. 5000 -(必须)
-w <working dir>        AFServer的工作路径 default "\tmp"
-b <block size>         blocksize for performing lookup,  default 256
-t <threshold>          threshold for performing lookup, default 0.015
-n <threads>            number of worker threads, default is 60
-i <index name>         path and name of index file -(必须)

例如: ./afserverd -p 5000 -i /home/soleo/test -n 2


Extra Information
========================

build.sh中的具体步骤

1. AFServer 依赖于 3个函数库，因此需要先在服务器上安装 ZeroMQ 、 table 和 phashaudio 3个库。

  编译时使用在AcousticFingerServer/libs目录中的makefile。 编译顺序为 ZeroMQ ；table ；phashaudio。

2. 使用AcousticFingerServer目录下的makefile进行编译。

3. 新建POSID.cfg，设定初始值为0

4. 拷贝编译后的文件到安装目录