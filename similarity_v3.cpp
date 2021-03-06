#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <cmath>
#include <cctype>
#include "similarity.h"
#include "utility.h"

namespace {
//
// 下面几个经验常量来自论文：《基于《知网》的词汇语义相似度计算》
//
    const float ALFA = 1.6;
    const float DELTA = 0.2;
    const float GAMA = 0.2;
    const float BETA[4] = {0.25, 0.45, 0.17, 0.13};

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
    if (!loadSememeOffspring()) {
        util::log("[ERROR] load offspring failed.");
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

//    取概念相似度的最大值
    float max = 0;
    float tmp = 0;
    for (size_t i = 0; i < sw1->size(); i++) {
        for (size_t j = 0; j < sw2->size(); j++) {
            tmp = calcGlossarySim(sw1->at(i), sw2->at(j));
            max = std::max(max, tmp);
        }
    }

    return max;
}


bool WordSimilarity::SememeElement::parse(const std::string &line) {
    if (line.empty()) return false;

    std::vector <std::string> items;
    util::strtok(items, line, "\t ");


    if (items.size() == 3 || items.size() == 4) {
        this->id = atol(items[items.size() - 3].c_str());
        //printf ("%d",this->id);


        this->father = atol(items[items.size() - 1].c_str());
        this->num_of_offspring = 0;
        //if (this->father==this->id) std::cout<<this->id<<std::endl;

        parseZhAndEn(items[items.size() - 2], &this->sememe_zh, &this->sememe_en);

        //util::log("[TRACE] %d, %s, %d", id, sememe_zh.c_str(), father);
        //std::cout << this->id << " " << this->sememe_en << " " << this->father << std::endl;
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
                this->s_other.push_back(this->s_first);
            }

            for (size_t i = 0; i < sememes.size(); i++) {
                if (0 == i && firstdone)
                    continue;

                char firstletter = sememes[i][0];

                std::string temp;
                parseZhAndEn(sememes[i], &temp);

                if ('(' == firstletter) {
                    this->s_other.push_back(temp);
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

                this->s_other.push_back(temp);
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
            ele->num_of_offspring = 0;
            if (ele->parse(line)) {
                sememetable_[ele->id] = ele;
                sememeindex_zn_[ele->sememe_zh] = ele;
            }
        }
    }
    in.close();
    return true;
}

bool WordSimilarity::loadSememeOffspring() {
    SememeTable::iterator it;

    for (it = sememetable_.begin(); it != sememetable_.end(); it++) {
        updateNumofOffspring(it);
        //count++;
    }
    return true;
}

bool WordSimilarity::updateNumofOffspring(SememeTable::iterator it) {
    int temp = it->first;
    while (true) {
        sememetable_[temp]->num_of_offspring = sememetable_[temp]->num_of_offspring + 1;
        if (sememetable_[temp]->father == sememetable_[temp]->id)
            break;
        else
            temp = sememetable_[temp]->father;
    }
// std::cout << sememetable_[1]->id;
    return true;
}

//bool WordSimilarity::loadSememeOffspring() {
//    for (int i = 0; i <= 1617; i++)
//        updateNumofOffspring(i);
//}

//bool WordSimilarity::updateNumofOffspring(int i) {
//    SememeElement *temp = sememetable_[i];
//    while (true) {
//        temp->num_of_offspring++;
//        if (temp->father == -1)
//            break;
//        else
//            temp = sememetable_[temp->father];
//    }
//}

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
    if (!in.good()) return false;

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


float WordSimilarity::calcGlossarySim(GlossaryElement *w1, GlossaryElement *w2) {
    if (!w1 || !w2) return 0.0;

    if (w1->solid != w2->solid) return 0.0;

    float sim1 = calcSememeSimFirst(w1, w2);
    float sim2 = calcSememeSimOther(w1, w2);
    float sim3 = calcSememeSimRelation(w1, w2);
    float sim4 = calcSememeSimSymbol(w1, w2);

    float sim = BETA[0] * sim1 +
                BETA[1] * sim2 +
                BETA[2] * sim1 * sim2 * sim3 +
                BETA[3] * sim1 * sim2 * sim3 * sim4;
//    printf("%f\n", sim2);

    return sim;
}


float WordSimilarity::calcSememeSimFirst(GlossaryElement *w1, GlossaryElement *w2) {
    return calcSememeSim(w1->s_first, w2->s_first);
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

// 计算两个【第一】基本义原之间的相似度
float WordSimilarity::calcSememeSim(const std::string &w1, const std::string &w2) {

    if (w1.empty() && w2.empty())
        return 1.0;
    if (w1.empty() || w2.empty())
        return DELTA;
    if (w1 == w2)
        return 1.0;

//int d = calcSememeDistance(w1, w2);

    SememeElement *s1 = getSememeByZh(w1);
    SememeElement *s2 = getSememeByZh(w2);

    if (!s1 || !s2)
        return -1.0;
    else {
        int iscommonfather = locateSememe(s1, s2);
        if (iscommonfather == -1)
            return 0.0;
        else {
            int father = getCommom(s1->id, s2->id);
            //printf("%ds%ds%d", s1->num_of_offspring, s2->num_of_offspring, sememetable_[father]->num_of_offspring);
            int sum = sememetable_[iscommonfather]->num_of_offspring;

            //std::cout<<sememetable_[father]->num_of_offspring<<" "<<s1->num_of_offspring<<" "<<s2->num_of_offspring<<std::endl;
            float i1 = 2 * log(1.0 * sememetable_[father]->num_of_offspring / ((sum + 1) * 1.0));
            float i2 = log(1.0 * s1->num_of_offspring / ((sum + 1) * 1.0));
            float i3 = log(1.0 * s2->num_of_offspring / ((sum + 1) * 1.0));

            float sim = i1 / (i2 + i3);
            return sim;
        }
        //return ALFA / (ALFA + d);
    }
}

int WordSimilarity::getCommom(int seme_id1, int seme_id2) {
    int current_father1 = seme_id1;
    int current_father2 = seme_id2;

    while (current_father1 != current_father2) {
        if (current_father1 > current_father2)
            current_father1 = sememetable_[current_father1]->father;
        else
            current_father2 = sememetable_[current_father2]->father;
    }
    return current_father1;
}

int WordSimilarity::locateSememe(SememeElement *sememe1, SememeElement *sememe2) {
    int father_sememe1 = sememe1->id;
    while (sememetable_[father_sememe1]->father != father_sememe1) {
        father_sememe1 = sememetable_[father_sememe1]->father;
    }
    int father_sememe2 = sememe2->id;
    while (sememetable_[father_sememe2]->father != father_sememe2) {
        father_sememe2 = sememetable_[father_sememe2]->father;
    }
    if (father_sememe1 == father_sememe2) return father_sememe1;
    else return -1;

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

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        printf("usage:%s word1 word2\n", argv[0]);
        return 1;
    }

    if (!WordSimilarity::instance()->init()) {
        util::log("[ERROR] init failed!!");
        return 1;
    }

    printf("[sim] %s - %s : %f\n", argv[1], argv[2],
           WordSimilarity::instance()->calc(argv[1], argv[2]));
}

#endif
