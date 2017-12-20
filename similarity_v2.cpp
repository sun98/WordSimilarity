#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <cctype>
#include "similarity.h"
#include "utility.h"

namespace {
    //
    // 下面几个经验常量来自论文：《基于《知网》的词汇语义相似度计算》
    //
    const float ALPHA = 1.6;
    const float EPSILON = 2.0;
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


WordSimilarity *WordSimilarity::instance() {
    static WordSimilarity ws;
    return &ws;
}

bool WordSimilarity::init(const char *sememefile, const char *glossaryfile) {
    if (!loadSememeTable(sememefile)) {
        util::log("[ERROR] %s load failed.", sememefile);
        return false;
    }
    if (!loadGlossary(glossaryfile)) {
        util::log("[ERROR] %s load failed.", glossaryfile);
        return false;
    }

    return true;
}

// 计算两个词的相似度
float WordSimilarity::calc(const std::string &w1, const std::string &w2) {
    if (w1 == w2)
        return 1.0;

//    得到word1和word2的概念
    GlossaryElements *sw1 = getGlossary(w1);
    GlossaryElements *sw2 = getGlossary(w2);

    if (!sw1 || !sw2 || !sw1->size() || !sw2->size())
        return -2.0;


//    for (int k = 0; k < sw1->size(); ++k) {
//        sw1->at(k)->dump();
//    }
//
//    for (int k = 0; k < sw2->size(); ++k) {
//        sw2->at(k)->dump();
//    }

//    取概念相似度的最大值->取第一基本义原相似度最大的概念为标准
//
    int pairSize = 2 * std::min(sw1->size(), sw2->size());
//    printf("size:%d, size1:%d, size2:%d\n", pairSize, sw1->size(), sw2->size());
    int pairCurSize = 0;
    int *maxPairs = new int[pairSize];
    bool flag = true;
    int maxI, maxJ;
    float maxFirst = -1, tmp;
    for (int l = 0; l < pairSize / 2; ++l) {
        maxFirst = -1;
        for (size_t i = 0; i < sw1->size(); ++i) {
            for (size_t j = 0; j < sw2->size(); ++j) {
                flag = true;
                for (int k = 0; k < pairCurSize; ++k) {
                    if (k % 2 == 0 && maxPairs[k] == i ||
                        k % 2 == 1 && maxPairs[k] == j)
                        flag = false;
                }
//                printf("cursize:%d, i:%d, j:%d, flag:%d\n", pairCurSize, i, j, flag);
                if (flag) {
                    tmp = calcSememeSimFirst(sw1->at(i), sw2->at(j));
                    if (maxFirst < tmp) {
                        maxFirst = tmp;
                        maxI = i;
                        maxJ = j;
                    }
                }
            }
        }
        maxPairs[pairCurSize++] = maxI;
        maxPairs[pairCurSize++] = maxJ;
//        printf("pair:%d, %d\n", maxI, maxJ);
    }
    float upper = 0, below = 0;
    for (int m = 0; m < pairCurSize / 2; ++m) {
        int sememeNum = 0;
        float glossarySim = 0;
        if (sw1->at(maxPairs[2 * m])->s_first != "") sememeNum++;
        if (!sw1->at(maxPairs[2 * m])->s_other.empty()) sememeNum++;
        if (!sw1->at(maxPairs[2 * m])->s_relation.empty()) sememeNum++;
        if (!sw1->at(maxPairs[2 * m])->s_symbol.empty()) sememeNum++;
        if (sw2->at(maxPairs[2 * m + 1])->s_first != "") sememeNum++;
        if (!sw2->at(maxPairs[2 * m + 1])->s_other.empty()) sememeNum++;
        if (!sw2->at(maxPairs[2 * m + 1])->s_relation.empty()) sememeNum++;
        if (!sw2->at(maxPairs[2 * m + 1])->s_symbol.empty()) sememeNum++;
        glossarySim = calcGlossarySim(sw1->at(maxPairs[2 * m]), sw2->at(maxPairs[2 * m + 1]));
        upper += sememeNum * glossarySim;
        below += sememeNum;
    }

    return upper / below;
}


bool WordSimilarity::SememeElement::parse(const std::string &line) {
    if (line.empty()) return false;

    std::vector <std::string> items;
    util::strtok(items, line, "\t ");
    if (items.size() == 3) {
        this->id = atol(items[0].c_str());
        this->father = atol(items[2].c_str());
        parseZhAndEn(items[1], &this->sememe_zh, &this->sememe_en);

        //util::log("[TRACE] %d, %s, %d", id, sememe_zh.c_str(), father);
        return true;
    }
    return false;
}

bool WordSimilarity::GlossaryElement::parse(const std::string &text) {
    std::string line = text;

    if (line.empty()) return false;

    std::vector <std::string> items;
    util::strtok(items, line, "\t ");
    if (items.size() == 3) {
        this->word = items[0];
        this->type = items[1];

        if (line[0] != '{') {
            this->solid = true;
        } else {
            this->solid = false;
            line = line.substr(1, line.size() - 2);
        }

        std::vector <std::string> sememes;
        util::strtok(sememes, items[2], ",");

        if (sememes.size() > 0) {
            bool firstdone = false;
            if (std::isalpha(sememes[0][0])) {
                parseZhAndEn(sememes[0], &this->s_first);
                firstdone = true;
            }

            for (size_t i = 0; i < sememes.size(); i++) {
                if (0 == i && firstdone)
                    continue;

                char firstletter = sememes[i][0];

                if ('(' == firstletter) {
                    this->s_other.push_back(sememes[i]);
                    continue;
                }

                size_t equalpos = sememes[i].find('=');
                if (equalpos != std::string::npos) {
                    std::string key = sememes[i].substr(0, equalpos);
                    std::string value = sememes[i].substr(equalpos + 1);
                    if (value.size() && value[0] != '(')
                        parseZhAndEn(value, &value);
                    this->s_relation.insert(std::make_pair(key, value));
                    continue;
                }

                if (!std::isalpha(firstletter)) {
                    std::string value = sememes[i].substr(1);
                    if (value.size() && value[0] != '(')
                        parseZhAndEn(value, &value);
                    this->s_symbol.insert(std::make_pair(firstletter, value));
                    continue;
                }

                this->s_other.push_back(sememes[i]);
            }
        }

        //dump();

        return true;
    }
    return false;
}

// 打印出词语的所有概念
void WordSimilarity::GlossaryElement::dump() {
    std::cout << word << ",\t" << type << ",\t|first:" << s_first << "\t|other:";
    for (size_t i = 0; i < s_other.size(); i++)
        std::cout << s_other[i] << ",";

    std::cout << "\t|relation:";
    for (std::map<std::string, std::string>::iterator it = s_relation.begin(); it != s_relation.end(); ++it)
        std::cout << it->first << "=" << it->second << ",";

    std::cout << "\t|symbol:";
    for (std::map<char, std::string>::iterator it = s_symbol.begin(); it != s_symbol.end(); ++it)
        std::cout << it->first << "=" << it->second << ",";

    std::cout << std::endl;
}

// 加载义原表
bool WordSimilarity::loadSememeTable(const char *filename) {
    std::ifstream in;
    in.open(filename, std::ios::in);
    if (!in.good())return false;

    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        if (!line.empty()) {
            SememeElement *ele = new SememeElement;
            if (ele->parse(line)) {
                sememetable_[ele->id] = ele;
                sememeindex_zn_[ele->sememe_zh] = ele;
            }
        }
    }
    in.close();
    return true;
}

WordSimilarity::SememeElement *WordSimilarity::getSememeByID(int id) {
    SememeTable::iterator it = sememetable_.find(id);
    if (it != sememetable_.end())
        return it->second;
    return NULL;
}

WordSimilarity::SememeElement *WordSimilarity::getSememeByZh(const std::string &word) {
    SememeIndex::iterator it = sememeindex_zn_.find(word);
    if (it != sememeindex_zn_.end())
        return it->second;
    return NULL;
}

bool WordSimilarity::loadGlossary(const char *filename) {
    std::ifstream in;
    in.open(filename, std::ios::in);
    if (!in.good())return false;

    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        if (!line.empty()) {
            GlossaryElement *ele = new GlossaryElement;
            if (ele->parse(line)) {
                glossarytable_[ele->word].push_back(ele);
            }
        }
    }
    in.close();
    return true;
}

WordSimilarity::GlossaryElements *WordSimilarity::getGlossary(const std::string &word) {
    GlossaryTable::iterator it = glossarytable_.find(word);
    if (it != glossarytable_.end())
        return &it->second;
    return NULL;
}

// 计算概念相似度
float WordSimilarity::calcGlossarySim(GlossaryElement *w1, GlossaryElement *w2) {
    if (!w1 || !w2) return 0.0;

    if (w1->solid != w2->solid) return 0.0;

    float sim1 = calcSememeSimFirst(w1, w2);
    float sim2 = calcSememeSimOther(w1, w2);
    float sim3 = calcSememeSimRelation(w1, w2);
    float sim4 = calcSememeSimSymbol(w1, w2);

//    float sim = BETA[0] * sim1 +
//                BETA[1] * sim1 * sim2 +
//                BETA[2] * sim1 * sim2 * sim3 +
//                BETA[3] * sim1 * sim2 * sim3 * sim4;
    float sim = BETA[0] * sim1 +
                BETA[1] * sim2 +
                BETA[2] * sim3 +
                BETA[3] * sim4;

    return sim;
}

// 计算第一基本义原，仍然使用alpha/(alpha+dist(w1,w2))的方法
float WordSimilarity::calcSememeSimFirst(GlossaryElement *w1, GlossaryElement *w2) {
    std::string word1, word2;
    word1 = w1->s_first;
    word2 = w2->s_first;
    if (word1.empty() && word2.empty())
        return 1.0;
    if (word1.empty() || word2.empty())
        return DELTA;
    if (word1 == word2)
        return 1.0;

    int d = calcSememeDistance(word1, word2);
    if (d >= 0)
        return ALPHA / (ALPHA + d);
    else
        return -1.0;
//    return calcSememeSim(w1->s_first, w2->s_first);
}

float WordSimilarity::calcSememeSimOther(GlossaryElement *w1, GlossaryElement *w2) {
    if (w1->s_other.empty() && w2->s_other.empty())
        return 1.0;

    float sum = 0.;
    float maxTemp = 0.;
    float temp = 0.;

    for (size_t i = 0; i < w1->s_other.size(); ++i) {
        maxTemp = -1.0;
        temp = 0.;

        for (size_t j = 0; j < w2->s_other.size(); ++j) {
            temp = 0.0;
            if (w1->s_other[i][0] != '(' && w2->s_other[j][0] != '(') {
                temp = calcSememeSim(w1->s_other[i], w2->s_other[j]);
            } else if (w1->s_other[i][0] == '(' && w2->s_other[j][0] == '(') {
                if (w1->s_other[i] == w2->s_other[j])
                    temp = 1.0;
                else
                    maxTemp = 0.0;
            } else {
                temp = GAMA;
            }

            if (temp > maxTemp)
                maxTemp = temp;
        }

        if (maxTemp == -1.0) //there is no element in w2->s_other
            maxTemp = DELTA;

        sum = sum + maxTemp;
    }

    if (w1->s_other.size() < w2->s_other.size())
        sum = sum + (w2->s_other.size() - w1->s_other.size()) * DELTA;

    return sum / std::max(w1->s_other.size(), w2->s_other.size());
}

float WordSimilarity::calcSememeSimRelation(GlossaryElement *w1, GlossaryElement *w2) {
    if (w1->s_relation.empty() && w2->s_relation.empty())
        return 1.0;

    float sum = 0.;
    float maxTemp = 0.;
    float temp = 0.;
    SememesRelation::const_iterator it1, it2;

    for (it1 = w1->s_relation.begin(); it1 != w1->s_relation.end(); ++it1) {
        maxTemp = 0.;
        temp = 0.;

        it2 = w2->s_relation.find(it1->first);
        if (it2 != w2->s_relation.end()) {
            if (it1->second[0] != '(' && it2->second[0] != '(') {
                temp = calcSememeSim(it1->second, it2->second);
            } else if (it1->second[0] == '(' && it2->second[0] == '(') {
                if (it1->second == it2->second)
                    temp = 1.0;
                else
                    maxTemp = 0.;
            } else {
                temp = GAMA;
            }

        } else
            maxTemp = DELTA;

        if (temp > maxTemp)
            maxTemp = temp;

        sum = sum + maxTemp;
    }

    if (w1->s_relation.size() < w2->s_relation.size())
        sum = sum + (w2->s_relation.size() - w1->s_relation.size()) * DELTA;

    return sum / std::max(w1->s_relation.size(), w2->s_relation.size());
}

float WordSimilarity::calcSememeSimSymbol(GlossaryElement *w1, GlossaryElement *w2) {
    if (w1->s_symbol.empty() && w2->s_symbol.empty())
        return 1.;

    float sum = 0.;
    float maxTemp = 0.;
    float temp = 0.;
    SememesSymbol::const_iterator it1, it2;

    for (it1 = w1->s_symbol.begin(); it1 != w1->s_symbol.end(); ++it1) {
        maxTemp = 0.;
        temp = 0.;

        it2 = w2->s_symbol.find(it1->first);
        if (it2 != w2->s_symbol.end()) {
            if (it1->second[0] != '(' && it2->second[0] != '(') {
                temp = calcSememeSim(it1->second, it2->second);
            } else if (it1->second[0] == '(' && it2->second[0] == '(') {
                if (it1->second == it2->second)
                    temp = 1.;
                else
                    maxTemp = 0.;
            } else {
                temp = GAMA;
            }

        } else
            maxTemp = DELTA;

        if (temp > maxTemp)
            maxTemp = temp;
        sum = sum + maxTemp;
    }

    if (w1->s_symbol.size() < w2->s_symbol.size())
        sum = sum + (w2->s_symbol.size() - w1->s_symbol.size()) * DELTA;

    return sum / std::max(w1->s_symbol.size(), w2->s_symbol.size());
}

// 计算两个义原之间的相似度(除第一基本义原外的)
float WordSimilarity::calcSememeSim(const std::string &w1, const std::string &w2) {
    int dist = calcSememeDistance(w1, w2);
    int depth1 = calcSememeDepth(w1), depth2 = calcSememeDepth(w2);
    if (dist >= 0 && depth1 >= 0 && depth2 >= 0) {
        int maxDepth = std::max(depth1, depth2), minDepth = std::min(depth1, depth2);
        float lambda = maxDepth / (maxDepth + minDepth);
        float temp = ALPHA * minDepth + EPSILON;
        return temp / (temp + lambda * (dist ^ 2));
    } else
        return -1.0;
}

// 计算义原在树结构中的深度
int WordSimilarity::calcSememeDepth(const std::string &w) {
    SememeElement *s = getSememeByZh(w);
    if (!s)
        return -1;
    int depth = 0, id = s->id, fatherId = s->father;
    while (id != fatherId) {
        id = fatherId;
        SememeElement *fatherNode = getSememeByID(fatherId);
        if (fatherNode) {
            fatherId = fatherNode->father;
        }
        depth++;
    }
    return depth;
}

// 计算两个义原在树结构中的距离
int WordSimilarity::calcSememeDistance(const std::string &w1, const std::string &w2) {
    SememeElement *s1 = getSememeByZh(w1);
    SememeElement *s2 = getSememeByZh(w2);
    if (!s1 || !s2)
        return -1;

    std::vector<int> fatherpath;

    int id1 = s1->id;
    int father1 = s1->father;

    while (id1 != father1) {
        fatherpath.push_back(id1);
        id1 = father1;
        SememeElement *father = getSememeByID(father1);
        if (father)
            father1 = father->father;
    }
    fatherpath.push_back(id1);


    int id2 = s2->id;
    int father2 = s2->father;
    int len = 0;
    std::vector<int>::iterator fatherpathpos;
    while (id2 != father2) {
        fatherpathpos = std::find(fatherpath.begin(), fatherpath.end(), id2);
        if (fatherpathpos != fatherpath.end()) {
            return fatherpathpos - fatherpath.begin() + len;
        }

        id2 = father2;
        SememeElement *father = getSememeByID(father2);
        if (father)
            father2 = father->father;
        ++len;
    }

    if (id2 == father2) {
        fatherpathpos = std::find(fatherpath.begin(), fatherpath.end(), id2);
        if (fatherpathpos != fatherpath.end()) {
            return fatherpathpos - fatherpath.begin() + len;
        }
    }

    return 20;
}


#ifdef _TEST_SIMILARITY

int main(int argc, const char* argv[])
{
    if (argc < 3)
    {
        printf("usage:%s word1 word2\n", argv[0]);
        return 1;
    }

    if (!WordSimilarity::instance()->init())
    {
        util::log("[ERROR] init failed!!");
        return 1;
    }

    printf("%s - %s : %f\n", argv[1], argv[2],
        WordSimilarity::instance()->calc(argv[1], argv[2]));
}

#endif