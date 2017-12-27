
#ifndef _SYNDIC_H
#define _SYNDIC_H
#include <string>
#include <map>
#include <vector>




class SynDic{
public:


    //单件
    static SynDic *instance();

    //初始化词林
    bool init(const char *dicfile="./cilin/dataset.txt");
    float calc (const std::string &w1 ,const std:: string &w2);
    float calcConceptSim(const std::string &w1 ,const std:: string &w2);


public:

    struct ConceptElement{
        bool parse(const std::vector<std::string> items,int i);


        std::string all;
        std::string firstType;
        std::string secondType;
        std::string thirdType;
        std::string wordGroup;
        std::string atomWordGroup;
        std::string symbol;


    };
    struct AtomWordGroupEle{
        int node_num;
        AtomWordGroupEle():node_num(0){}
    };
    struct WordGroupEle{
        std::map<std::string, AtomWordGroupEle*> atomWordGroupDic;
        int node_num;
        WordGroupEle():node_num(0){}
    };
    struct ThirdTypeEle{
        std::map<std::string, WordGroupEle*> wordGroupDic;
        int node_num;
        ThirdTypeEle():node_num(0){}
    };
    struct SecondTypeEle{
        std::map<std::string, ThirdTypeEle*> thirdTypeDic;
        int node_num;
        SecondTypeEle():node_num(0){}
    };
    struct FirstTypeEle{
        std::map<std::string, SecondTypeEle*> secondTypeDic;
        int node_num;
        FirstTypeEle():node_num(0){}
    };
    struct ConceptDic{
        std::map<std::string, FirstTypeEle*> firstTypeDic;
        int node_num;
        ConceptDic():node_num(0){}
    };
//    struct ConceptDic{
//        std::map<char, FirstTypeEle*> firstTypeDic;
//        int node_num;
//        ConceptDic():node_num(0){}
//    };
    typedef std::map<std::string,ConceptElement*> ConceptTable;
    //typedef std::map<std::string,FirstTypeEle*> ConceptDic;


protected:

    bool loadConceptTable(const char *filename);

    ConceptElement *getConceptByZh(const std::string &concept);

    //float calcConceptSim(const std::string &w1,const std::string &w2);

private:

    SynDic(){}

    ConceptTable concepttable_;
    ConceptDic   conceptdic_;



};

#endif
