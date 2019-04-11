# Introduction 

本文档目的是帮助选手快速上手所有源代码

## 代码说明

`./说明文档/`：目录下包含所有你应该知道的说明文档信息

`./DEBUG/`：目录下包含保存了调试信息的文件，需要打开DEBUG开关才能保存调试信息

`./Maps/`：目录下包含地图文件

`./Replay/`：目录下包含可播放的文件，使用`GamePlayer.py`时默认播放最新的文件，除非调用时加参数

`Arguments.py`：参数

`server.py`：本地服务器

`main.py`：逻辑主体

`playerAI.cpp`：选手需要填写的文件

## 使用方法

1. 填写`playerAI.cpp`与`playerAI.h`

2. 在代码所在的目录下终端/cmd中使用`make`

3. 运行`server.py`

4. 在经过漫长的等待之后，就会出现类似这样的信息，不过没关系，游戏已经正常结束，可以使用Ctrl+C强行结束掉

	```
	write to judger
	b'Fatal Python error: could not acquire lock for <_io.BufferedReader name=\'<stdin>\'> at interpreter shutdown, possibly due to daemon threads\n\nThread 0x00007fe273605700 (most recent call first):\n  File "main.py", line 190 in recvData\n  File "main.py", line 246 in run\n  File "/usr/lib/python3.6/threading.py", line 916 in _bootstrap_inner\n  File "/usr/lib/python3.6/threading.py", line 884 in _bootstrap\n\nCurrent thread 0x00007fe280037740 (most recent call first):\n'
	end judger
	```

5. 此时，可以将`./Replay/`中生成的`replay*.json`用播放器播放，或是使用`Gameplayer.py`（简略版播放器）

## 提示

1. 不要对`Arguments.py,0.json`进行任何变动
2. 输出调试信息，不要输出到标准输入输出中，即`printf/cout`，写入到文件里，推荐写入到`./DEBUG/`下
3. `main.py`中`PYGAME`设置为`True`可以运行时播放， `DEBUG`设置为True，会在`./DEBUG/`下生成一个对应`replay`编号的debug文件，可以进行阅读以便debug
4. 如果想要使用更多文件，需要修改`makefile`，请自行学习`makefile`的编写规则，不然只能修改`playerAI.h`与`playerAI.cpp`
5. 默认使用同一AI的两份copy进行对战，如果使用不同AI，请先后生成两份可执行文件（`main1.exe` `main2.exe`），并修改`server.py`最后几行中的指令

如果发现judger中有任何bug或漏洞，请在群里及时@游戏逻辑 徐天行，谢谢！

