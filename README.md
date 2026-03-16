# RR3TextureConvert

- 编写中...目标是全自动转换贴图格式。
- ptc.pvr不在考虑范围内，因为我没有苹果设备（以及苹果会自动清数据！）。 
- 目前可用转换etc->dxt。贴图相关后缀已经全部支持。（.dds, .dds.z, .dds.z.bin）

---

- still programming... Aim is automatically convert the texture types in a full data package (atc,etc,dxt.dds, and rgb.pvr).

- now can mank etc->dxt. and the exts(.dds, .dds.z, .dds.z.bin) are all supported. 

- ptc.pvr is not in the plan, because I have no ios device. (and iOS clear data, so I think its hard to play on iOS)

# 目前已经研究的其他内容：
- 更新弹窗跳过：删除文件 ```Android/data/com.ea.games.r3_(ext)/.depot/CC_Data/1008``` [链接/link](https://tieba.baidu.com/p/10558452395)
- skip the force update dialog: delete file ```Android/data/com.ea.games.r3_(ext)/.depot/CC_Data/1008``` (and makesure offline, DO NOT connect any network while launching the game)

---

- 欺骗下载进度检测：从```res.zip```里的一系列```asset_list_(...).txt```中构建出完整的 ```Android\data\com.ea.games.r3_(ext)\files\.depot\asset_list_base.txtCache.txt"``` [链接/link](https://tieba.baidu.com/p/10550302697)
- Trick to skip download: from```res.zip```(in the apk), get a series of txtfile looks like ```"asset_list_(...).txt"```. then connect them(the newline character should be linux \n ), then fix the header(12 bytes), make it same with the target file: ```Android\data\com.ea.games.r3_(ext)\files\.depot\asset_list_base.txtCache.txt"``` then replace it. now the game wont ask to download, but crash if missing files.
