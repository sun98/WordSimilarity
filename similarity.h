//
// Created by 汤同辉 on 2017/12/28.
//

#ifndef SIM_V2_SIMILARITY_H
#define SIM_V2_SIMILARITY_H
#ifndef _SIMILARITY_H
#define _SIMILARITY_H

#include <string>
#include <map>
#include <vector>
#include <math.h>

///////////////////////////////////////////////////////////////
//
// 词汇语义相似度计算器：(by david++ 2014/05/06)
//
// 具体算法实现，参见文献：《基于《知网》的词汇语义相似度计算》
//
// 用法：
//  WordSimilarity::instance()->init();
//  WordSimilarity::instance()->calc("十分", "犀利");
//
///////////////////////////////////////////////////////////////
class WordSimilarity {
public:
    //
    // 单件
    //
    static WordSimilarity *instance();


    //
    // 初始化义原和词汇表
    //

//    bool init(const char *sememefile = "/Users/tangtonghui/Documents/project/ClionProjects/sim_v2/hownet/WHOLE.DAT", const char *glossaryfile = "/Users/tangtonghui/Documents/project/ClionProjects/sim_v2/hownet/glossary.dat");
    bool init(const char *sememefile = "./hownet/WHOLE.DAT", const char *glossaryfile = "./hownet/glossary.dat");

    //
    // 计算两个词的语义相似度（返回值: [0, 1], -2:指定的词词典中不存在）
    //
    float calc(const std::string &w1, const std::string &w2);

public:
    // 义原条目
    struct SememeElement {
        bool parse(const std::string &text);

        int id;                  // 编号
        std::string sememe_en;   // 英文义原
        std::string sememe_zh;   // 中文义原
        int father;              // 父义原编号
        int num_of_offspring;    //子孙和
    };

    typedef std::map<int, SememeElement *> SememeTable;
    typedef std::map<std::string, SememeElement *> SememeIndex;

    typedef std::vector <std::string> SememesOther;
    typedef std::map <std::string, std::string> SememesRelation;
    typedef std::map<char, std::string> SememesSymbol;

    // 词汇表条目
    struct GlossaryElement {
        bool parse(const std::string &text);

        void dump();

        bool solid;                 // 实词/虚词
        std::string word;           // 词
        std::string type;           // 词性
        std::string s_first;        // 第一基本义原
        SememesOther s_other;       // 其他义原
        SememesRelation s_relation; // 关系义原
        SememesSymbol s_symbol;     // 符号义原
    };

    typedef std::vector<GlossaryElement *> GlossaryElements;
    typedef std::map <std::string, GlossaryElements> GlossaryTable;

protected:
    //
    // 加载义原文件
    //
    bool loadSememeTable(const char *filename);

    //
    // 加载词汇表
    //
    bool loadGlossary(const char *filename);

    //
    //录入义原树子孙信息
    bool loadSememeOffspring();


    //
    // getSememeByID - 根据编号获取义原
    // getSememeByZh - 根据汉词获取义原
    // getGlossary   - 获取词汇表中的词
    // updateNumofOffspring - 更新子孙和
    //
    SememeElement *getSememeByID(int id);

    SememeElement *getSememeByZh(const std::string &word);

    GlossaryElements *getGlossary(const std::string &word);

    bool updateNumofOffspring(SememeTable::iterator it);

    int locateSememe(SememeElement *sememe1, SememeElement *sememe2);


    //
    // calcGlossarySim    - 计算词汇表中两个词的相似度
    // calcSememeSim      - 计算两个义原之间的相似度
    // calcSememeDistance - 计算义原之间的距离(义原树中两个节点之间的距离)
    // calcSememeDepth    - 计算义原在树结构的深度
    //
    float calcGlossarySim(GlossaryElement *w1, GlossaryElement *w2);

    float calcSememeSim(const std::string &w1, const std::string &w2);

    int calcSememeDistance(const std::string &w1, const std::string &w2);

    int calcSememeDepth(const std::string &w);

    int getCommon(int seme_id1, int seme_id2);

    //
    // calcSememeSimFirst    - 计算第一基本义原之间的相似度
    // calcSememeSimOther    - 计算其他义原之间的相似度
    // calcSememeSimRelation - 计算关系义原之间的相似度
    // calcSememeSimSymbol   - 计算符号义原之间的相似度
    //
    float calcSememeSimFirst(GlossaryElement *w1, GlossaryElement *w2);

    float calcSememeSimOther(GlossaryElement *w1, GlossaryElement *w2);

    float calcSememeSimRelation(GlossaryElement *w1, GlossaryElement *w2);

    float calcSememeSimSymbol(GlossaryElement *w1, GlossaryElement *w2);

private:
    WordSimilarity() {}

    // 义原表
    SememeTable sememetable_;

    // 义原索引(中文)
    SememeIndex sememeindex_zn_;

    // 词汇表
    GlossaryTable glossarytable_;
};

#endif

#endif //SIM_V2_SIMILARITY_H
