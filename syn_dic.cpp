#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <cctype>
#include <cmath>

#include "syn_dic.h"
#include "utility.h"
#include "mathlib.h"

#define  MAX 90111
#define DP 23
namespace {
    //
    // 下面几个经验常量来自论文：《基于《知网》的词汇语义相似度计算》
    //
    const float ALFA = 1.6;
    const float DELTA = 0.2;
    const float GAMA = 0.2;
    const float BETA[4] = {0.5, 0.2, 0.17, 0.13};

    void parseZhAndEn(const std::string &text, std::string *zh, std::string *en = NULL) {
        std::vector <std::string> words;
        util::strtok(words, text, "|");
        if (words.size() == 2) {
            if (en) *en = words[0];
            if (zh) *zh = words[1];
        } else {
            if (en) *en = text;
            if (zh) *zh = text;
        }
    }
}


SynDic *SynDic::instance() {
    static SynDic sd;
    return &sd;
}


bool SynDic::init(const char *dicfile) {
    if (!loadConceptTable(dicfile)) {
        util::log("[ERROR] %s load failed.", dicfile);
        return false;
    }
    //loadConceptTable(dicfile);
    return true;
}

float SynDic::calc(const std::string &w1, const std::string &w2) {
    if (w1 == w2)
        return 1.0;

    return calcConceptSim(w1, w2);

}

float SynDic::calcConceptSim(const std::string &w1, const std::string &w2) {
    ConceptElement *ce1 = getConceptByZh(w1);
    ConceptElement *ce2 = getConceptByZh(w2);
    if (!ce1 || !ce2)return 0.32;
    else {
        float result1 = log(DP +
                            conceptdic_.firstTypeDic[ce1->firstType]->secondTypeDic[ce1->secondType]->thirdTypeDic[ce1->thirdType]->wordGroupDic[ce1->wordGroup]->atomWordGroupDic[ce1->atomWordGroup]->node_num);
        float result2 = log(DP +
                            conceptdic_.firstTypeDic[ce2->firstType]->secondTypeDic[ce2->secondType]->thirdTypeDic[ce2->thirdType]->wordGroupDic[ce2->wordGroup]->atomWordGroupDic[ce2->atomWordGroup]->node_num);
        //float result1=log((dp+conceptdic_.firstTypeDic[ce1->firstType]->secondTypeDic[ce1->secondType]->thirdTypeDic[ce1->thirdType]->wordGroupDic[ce1->wordGroup]->atomWordGroupDic[ce1->atomWordGroup]->node_num)*1.0/MAX);
        //
        //float result2=log((dp+conceptdic_.firstTypeDic[ce2->firstType]->secondTypeDic[ce2->secondType]->thirdTypeDic[ce2->thirdType]->wordGroupDic[ce2->wordGroup]->atomWordGroupDic[ce2->atomWordGroup]->node_num)*1.0/MAX);
        //
        //printf("%f\n",result1);
        //printf("%f\n",result2);

        float all;

        if (ce1->firstType != ce2->firstType)
            return (result1 + result2) / (4 * log(DP + MAX));
            //return 2.0*log(MAX*1.0/MAX)/(result1+result2);
        else {
            if (ce1->secondType != ce2->secondType)
                all = log(DP + conceptdic_.firstTypeDic[ce2->firstType]->node_num);
            else {
                if (ce1->thirdType != ce2->thirdType)
                    all = log(DP + conceptdic_.firstTypeDic[ce2->firstType]->secondTypeDic[ce2->secondType]->node_num);
                else {
                    if (ce1->wordGroup != ce2->wordGroup)
                        all = log(DP +
                                  conceptdic_.firstTypeDic[ce2->firstType]->secondTypeDic[ce2->secondType]->thirdTypeDic[ce2->thirdType]->node_num);
                    else {
                        if (ce2->atomWordGroup != ce1->atomWordGroup)
                            all = log(DP +
                                      conceptdic_.firstTypeDic[ce2->firstType]->secondTypeDic[ce2->secondType]->thirdTypeDic[ce2->thirdType]->wordGroupDic[ce2->wordGroup]->node_num);
                        else
                            return 1.0;

                    }
                }
            }
        }


        return (result1 + result2) / (2 * all);
        //return (2.0*all/(result1+result2));

    }


}

bool SynDic::ConceptElement::parse(const std::vector <std::string> items, int i) {

    this->firstType = items[0].substr(0, 1);


    this->secondType = items[0].substr(1, 1);
    this->thirdType = items[0].substr(2, 2);
    this->wordGroup = items[0].substr(4, 1);
    this->atomWordGroup = items[0].substr(5, 2);
    this->symbol = items[0].substr(7, 1);

    this->all = items[0];
    //printf("%s%s%s%s%s%s",(this->firstType, this->secondType.c_str(),this->thirdType.c_str(),this->wordGroup.c_str(), this->atomWordGroup.c_str(),this->symbol.c_str()));
    //std::cout<<this->firstType;
    return true;


}

bool SynDic::loadConceptTable(const char *filename) {
    std::ifstream in;
    int count = 0;
    FirstTypeEle *ftemp;
    SecondTypeEle *stemp;
    ThirdTypeEle *ttemp;
    WordGroupEle *wtemp;
    AtomWordGroupEle *atemp;
    in.open("./cilin/dataset.txt");
//    in.open("/Users/tangtonghui/Documents/project/ClionProjects/syndic/dataset.txt");
    if (!in.good())return false;

    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        if (!line.empty()) {
            std::vector <std::string> items;
            util::strtok(items, line, " ");
            ConceptElement *ele;
            for (int i = 0; i < items.size() - 1; i++) {
                ele = new ConceptElement;
                if (ele->parse(items, i))
                    count++;
                concepttable_[items[i + 1]] = ele;
                if (!conceptdic_.firstTypeDic[ele->firstType])
                    conceptdic_.firstTypeDic[ele->firstType] = new FirstTypeEle;
                ftemp = conceptdic_.firstTypeDic[ele->firstType];
                ftemp->node_num++;

                if (!ftemp->secondTypeDic[ele->secondType])
                    ftemp->secondTypeDic[ele->secondType] = new SecondTypeEle;
                stemp = ftemp->secondTypeDic[ele->secondType];
                stemp->node_num++;

                if (!stemp->thirdTypeDic[ele->thirdType])
                    stemp->thirdTypeDic[ele->thirdType] = new ThirdTypeEle;
                ttemp = stemp->thirdTypeDic[ele->thirdType];
                ttemp->node_num++;

                if (!ttemp->wordGroupDic[ele->wordGroup])
                    ttemp->wordGroupDic[ele->wordGroup] = new WordGroupEle;
                wtemp = ttemp->wordGroupDic[ele->wordGroup];
                wtemp->node_num++;

                if (!wtemp->atomWordGroupDic[ele->atomWordGroup])
                    wtemp->atomWordGroupDic[ele->atomWordGroup] = new AtomWordGroupEle;
                atemp = wtemp->atomWordGroupDic[ele->atomWordGroup];
                atemp->node_num++;


                // conceptdic_.firstTypeDic[ele->firstType]->secondTypeDic[ele->secondType]->thirdTypeDic[ele->thirdType]->node_num++;
            }
        }
    }
    in.close();
    //printf("%d",count);
    return true;
}

SynDic::ConceptElement *SynDic::getConceptByZh(const std::string &concept) {
    ConceptTable::iterator it = concepttable_.find(concept);
    if (it != concepttable_.end())
        return it->second;
    return NULL;
}

float execCsv() {
    std::ifstream ifs("./results/combined_zh_cilin.csv");
    std::ofstream ofs("./results/combined_zh_syn.csv");
    std::string word1, word2, human_result, blank;

    float *human = new float[346], *v2_results = new float[346];
    double *corr = new double(0.0);
    int index = 0;

    while (!ifs.eof()) {
        getline(ifs, word1, ',');
        getline(ifs, word2, ',');
        getline(ifs, human_result, ',');
        getline(ifs, blank, '\n');
//        printf("%s, %s, %s\n", word1.c_str(), word2.c_str(), human_result.c_str());
        float v2_result = SynDic::instance()->calc(word1, word2);
        human[index] = (float) atof(human_result.c_str());
        v2_results[index++] = v2_result;
        ofs << word1 << "," << word2 << "," << human_result << "," << v2_result << ",\n";
    }
    ifs.close();
    ofs.close();
    CCalculate calculate;
    calculate.GetCorrCoef(human, v2_results, 346, *corr);
    return (float) *corr;
}


#ifdef _TEST_SYN_DIC

int main(int argc, const char* argv[]) {
    if (!SynDic::instance()->init()) {
        util::log("[ERROR] init failed!!");
        return 1;
    }

    if (argc < 3) {
        printf("%f", execCsv());
        return 1;
    }
    printf("%s - %s : %f\n", argv[1], argv[2],
        SynDic::instance()->calc(argv[1], argv[2]));
    return 1;
}

#endif
