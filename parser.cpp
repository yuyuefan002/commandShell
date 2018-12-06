#include "parser.h"

/*
  strList2charstarlist

  This function will transfer vector<string> to char**

  INPUT
  str: the string array in vector<string> type

  OUTPUT
  res: the pointer pointing to the space of string array

*/
char ** Parser::strList2charStarList(std::vector<std::string> & str) {
  char ** res = new char *[str.size() + 1];
  // pointing at each string
  for (size_t i = 0; i < str.size(); i++) {
    res[i] = (char *)str[i].c_str();
  }
  // explicitly end the array
  res[str.size()] = nullptr;
  return res;
}

/*
  trim
  
  This function will trim the string at the left of string

  INPUT
  s:the string to be trimed
  split:the char need to be deleted
  
  OUTPUT
  s:trimed string
  
 */
std::string Parser::trim(std::string s, char split) {
  // erase the substring in s starting from 0 to the first char not split
  s.erase(0, s.find_first_not_of(split));
  return s;
}

/*
  name: readFirstWord
  input: s(the string to read), enBackSlash(enable '\ ' exist), split(the char to split word)
  output: the first word in string
  description: read the first word in the string
 */
std::string Parser::readFirstWord(std::string s, int enBackSlash, char split, int rdiFileMode) {
  size_t end = 0;
  // find the first split(char)
  while (end < s.size() && s[end] != split) {
    if (rdiFileMode && (s[end] == '<' || s[end] == '>')) {
      break;
    }
    if (enBackSlash && end + 1 < s.size() && s[end] == '\\') {
      end++;
    }
    end++;
  }
  // get the substr from start to split
  return s.substr(0, end);
}

/*
  name: isnum
  input: s(the string to be judged)
  output: bool
  description: judge whether the string is num, if yes,return true
 */
bool Parser::isnum(std::string s) {
  for (char c : s) {
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

/*
  name: delFirstWord
  input: s(the string to be revised), split(the char to split word)
  output: trimed string
  description: delete the first word in the string
 */
std::string Parser::delFirstWord(std::string s, char split) {
  // delete all spaces before any sign
  s = trim(s);
  // delete the first word
  s.erase(0, s.find_first_of(split));
  // delete all the following spaces before any sign
  s = trim(s, split);
  return s;
}

/*
  name: inc
  input: v(string represents the value)
  output: the string represeting original number+1
  description: the number in string format add 1 and return
 */
std::string Parser::inc(std::string v) {
  // get the value
  int val = atoi(v.c_str());
  // value=value+1;
  std::stringstream ss;
  // never overflow
  if (val != INT_MAX) {
    ss << val + 1;
  }
  else
    ss << val;
  return ss.str();
}
/*
  name: delSubStr
  input: s(the string to be revised), sub(the substring pattern)
  output: revised string
  description: delete all the substring in the string
 */
std::string Parser::repSubStr(std::string s, std::string sub, std::string newsub) {
  // find the position of substring
  size_t len = sub.size();
  size_t p = s.find(sub);
  // until no substring inside
  while (p != std::string::npos) {
    // delete it
    s.replace(p, len, newsub);
    // get a new position
    p = s.find(sub);
  }
  return s;
}

/*
  parseEnvPath

  This function will read the environment path and store them into list

  OUTPUT
  envp:list of paths

 */
std::vector<std::string> Parser::parseEnvPath() {
  std::string path;
  // read environment path
  if (getenv("PATH") != nullptr)
    path = getenv("PATH");
  std::vector<std::string> envp;
  // split them by ":"
  while (path.size()) {
    std::string word = readFirstWord(path, 0, ':');
    envp.push_back(word);
    path = delFirstWord(path, ':');
  }
  return envp;
}

/*
  rmBackSlash

  This function will delete any first \ in two letters

  INPUT
  s: the string possible contains \

  OUTPUT
  res: delete every escape character

 */
std::string rmBackSlash(std::string s) {
  std::string res;
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == '\\' && i + 1 < s.size()) {
      ++i;
    }
    res += s[i];
  }
  return res;
}

/*
  subHomePathfortilde

  This function will substitute ~ by the home patn

  INPUT
  s: the string possible contains '~'

  OUTPUT
  res: the revised string

 */
std::string subHomePathforTilde(std::string s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == '~') {
      s.replace(i, 1, getenv("HOME"));
    }
  }
  return s;
}

/*
  name: parseCmd

  This function will read the whole command and split them into arguments and store
  
  INPUT
  command: the commands only include args 
  
  OUTPUT
  res:list of argvs
 
 */
std::vector<std::string> Parser::parseCmd(std::string command) {
  std::vector<std::string> res;
  while (command.size()) {
    // read each argument
    std::string word = readFirstWord(command, 1);
    size_t pos = word.size();
    // remove back slash
    word = rmBackSlash(word);
    // substitute tilde by home path
    word = subHomePathforTilde(word);
    // store it
    res.push_back(word);
    command.erase(0, pos);
    command = trim(command);
  }
  return res;
}
