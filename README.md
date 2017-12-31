# WordSimilarity
Optimization of Chinese word similarity calculation

- 编译：
    - 原始版本 `make sim`
    - 修改版知网 `make simv2`
    - 基于信息论的词林 `make simv3`
- 运行：
    - 原始版本 `./sim 词语1 词语2`
    - 修改版知网 `./sim2 词语1 词语2`
    - 基于信息论的词林 `./syn_dic 词语1 词语2`

- 批量测试（miller人工集）
    - 原始版本 `make origin`
    - 修改版知网 `make v2`
    - 基于信息论的词林 `make v3`
    
- 批量测试（300多词的词集,打印出相关系数）
    - 原始版本 `make csv_v1`
    - 修改版知网 `make csv_v2`
    - 基于信息论的词林 `make csv_syn_dic`
