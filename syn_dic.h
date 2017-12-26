#include <string>
#include <map>
#include <vector>




class SynDic{
public:


  //单件
  static SynDic *instance();

  //初始化词林
  bool init(const char *dicfile="./cilin/dataset.txt")
  float calc (const std::string &w1 ,const std:: string &w2);


public:
  struct ConceptElement{
    bool parse(const std::string &text);



    
  }
}
