#ifndef __PARSER_H__
#define __PARSER_H__
#include <climits>
#include <sstream>
#include <string>
#include <vector>
/*
  class Parser

  method:

         inc             treat string as a number and add 1
         trim            delete all the starting chars specified by split
	 readFirstWord   read the first word in string
         delFirstWord    delete the first word in string
	 delSubStr       delete the substring in string
         isnum           judge whether the string indicats a number
	 parseCmd        parse the whole command
	 parseEnvpath    path the environment path
	 strList2charStarList
*/
class Parser
{
 private:
 public:
  bool isnum(std::string s);
  std::string inc(std::string v);
  std::vector<std::string> parseEnvPath();
  std::string trim(std::string s, char split = ' ');
  std::string delFirstWord(std::string s, char split = ' ');
  std::vector<std::string> parseCmd(std::string command);
  char ** strList2charStarList(std::vector<std::string> & str);
  std::string repSubStr(std::string s, std::string sub, std::string newsub = "");
  std::string readFirstWord(std::string s,
                            int enBackSlash = 0,
                            char split = ' ',
                            int rdiFileMode = 0);
};
#endif
