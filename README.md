# naive-file-transcoder

**因为我的毛选1-7被百度云和谐了，所以我决定写个这玩意。**

---------

给你的文件套个壳，不影响原文件大小。  
原理就是二进制流读入文件，每个字节异或某个值，再输出到目标文件。

--------

#### 用法

- 终端：`./transcoder filename` filename为文件名，自动识别文件后缀，为`.trs`则解码，否则编码。

- 还可以直接把文件在文件资源管理器里面拖到 transcoder 上，提示 `用 transcoder 打开` 时松手。

-------

自动分卷，大小2GB。（想改的话就改代码里的 `split_limit` 变量，单位MB）  
编码时自动拆成若干个文件，格式 `filename.p*.trs`。  
解码时输入任意一个部分的文件名即可。

----

#### 更新

v1.1：  
添加了元数据（`filename.meta`）  
存储程序版本以及 key 和文件的 crc32 值  
可以判断文件是否损坏和 key 是否正确。

-----

#### 特别鸣谢

[百度网盘](https://pan.baidu.com/)

ini 库来自 [hydropek/libini](https://github.com/hydropek/libini)  
crc-32 库来自 [stbrumme/hash-library](https://github.com/stbrumme/hash-library)

