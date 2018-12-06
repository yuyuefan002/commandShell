#include "interface.h"

/*
  min

  This function return the min value between a,b
  
  INPUT
  a: number a
  b: number b

  OUTPUT
  the smaller one between a and b
 */
size_t min(size_t a, size_t b) {
  if (a < b)
    return a;
  return b;
}

/*
  first_pos_of_rdi

  This function return the first pos of rdi

  INPUT
  the user input command

  OUTPUT
  the pos of first redirection sign
 */
size_t first_pos_of_rdi(std::string command) {
  size_t pos = std::string::npos;
  if ((pos = command.find("2>")) != std::string::npos) {
    if (pos - 1 >= 0 && command[pos - 1] == ' ') {
    }
    else
      pos = std::string::npos;
  }
  pos = min(command.find(">"), min(pos, command.find("<")));
  return pos;
}

/*
  readFirstVar

  This function will read the first var name in the string

  INPUT
  s:a string contains var name

  OUTPUT
  var: the first var name in the given string

 */
std::string readFirstVar(std::string s) {
  std::string var;
  // the var name starting after '$'
  size_t start = s.find("$") + 1;
  size_t end = start;
  // var name only includes '-' or number or alpha
  while (isalnum(s[end]) || s[end] == '_')
    ++end;
  int len = end - start;
  var = s.substr(start, len);
  return var;
}

/*
  subVar4Value
  
  This function will using the value to substitute vars in the string

  INPUT
  S:string contains vars
  
  OUTPUT
  S:translated string
  
 */
std::string Interface::subVar4Value(std::string S) {
  std::string res;
  while (S.find("$") != std::string::npos && S.find("$") != S.size() - 1) {
    size_t start = S.find("$");
    // add the string before "$" into result, since they are ordinary strings
    res += S.substr(0, start);
    // if there is a '\' before '$' it means '$' is just a sign, it will not indicate there is a var behind
    if (start - 1 >= 0 &&
        S[start - 1] == '\\') {  // || (start + 1 < S.size() && S[start + 1] == ' ')) {
      S = S.substr(start + 1);
      res += "$";
      continue;
    }
    // read the var, add value of var into result.
    std::string var = readFirstVar(S);
    S = S.substr(start + var.size() + 1);
    res += vars[var];
  }
  // add left strings into result
  res += S;
  return res;
}

/*
  execSysFunc
  
  This function will execute the shell functions, cd, set, export, inc

  INPUT
  argv: the command arguments
  
  OUTPUT
  bool: indicating run shell command or not
  
 */
bool Interface::execSysFunc(std::vector<std::string> argv) {
  if (argv.size() == 0) {
    return false;
  }
  if (argv[0] == "cd") {
    exec_cd(argv);
  }
  else if (argv[0] == "set") {
    if (exec_set(argv) == -1) {
      std::cerr << "set fail\n";
    }
  }
  else if (argv[0] == "export") {
    if (exec_export(argv) == -1) {
      std::cerr << "export fail\n";
    }
  }
  else if (argv[0] == "inc") {
    if (exec_inc(argv) == -1) {
      std::cerr << "inc fail\n";
    }
  }
  else {
    return false;
  }
  return true;
}

/*
  clearAllCmd
  
  This function will delete all the data, therefore, no command will be executed

*/
void Interface::clearAllCmd() {
  argvs.clear();
  envPath.clear();
}

/*
  readRdiFilename

  This function will read the filename specified after redirection sign

  INPUT
  command: the command contains redirection and the filename
  pos: the start pos of filename

  OUTPUT
  fn: the filename specified by redirection

*/
std::string Interface::readRdiFilename(std::string command, size_t pos) {
  std::string fn = command.substr(pos);
  fn = parser.trim(fn);
  fn = parser.readFirstWord(fn, 1, ' ', 1);
  return fn;
}

/*
  nextRdi
  
  This function will return the substring which is leading by redirection sign or npos

  INPUT
  command:The string only includes rdi

  OUTPUT
  the demanding string
 */
std::string nextRdi(std::string command) {
  size_t pos = first_pos_of_rdi(command);
  if (pos == std::string::npos)
    return "";
  return command.substr(pos);
}

/*
  detectRdi

  This function will detect the possible redirection in the command

  INPUT
  command: the command possibly containing redirection
  dir: the current command's redirection
  filename:the current command's redirection filename

  OUTPUT
  command: the command being deleted the current redirection
 */
std::string Interface::detectRdi(std::string command,
                                 std::vector<int> & dir,
                                 std::vector<std::string> & filename) {
  //size_t pos;
  int oneDir = -1;
  std::string oneFilename;
  //if ((pos = command.find("<")) != std::string::npos) {
  if (command[0] == '<') {
    oneDir = 1;
    oneFilename = readRdiFilename(command, 1);
  }
  // redirect err output
  //else if ((pos = command.find("2>")) != std::string::npos) {
  else if (command[0] == '2') {
    oneDir = 2;
    oneFilename = readRdiFilename(command, 2);
  }

  // redirect output
  //else if ((pos = command.find(">")) != std::string::npos) {
  else if (command[0] == '>') {
    oneDir = 0;
    oneFilename = readRdiFilename(command, 1);
  }
  // no redirection
  else {
    oneDir = -1;
    oneFilename.clear();
  }
  dir.push_back(oneDir);
  filename.push_back(oneFilename);
  // revise the command so that it does not contain any redirection
  return nextRdi(command.substr(2));
}

/*
  init

  This function isn't elegant I think, but is required in my code....
  Init every variable to make sure nothing strange happend.
*/
void Interface::init() {
  command.clear();
  argvs.clear();
  dirs.clear();
  filenames.clear();
}

/*
  detectPipes

  This function can split the  command by pipe and store them into argvs

  INPUT
  command: the command input by user

  OUTPUT
  0/-1: return -1 when failure.
 */
int Interface::detectPipes(std::string command) {
  size_t pos = command.find("|");
  while (pos != std::string::npos) {
    // trim all leading space
    // read the command before |
    std::string pipeCommand = command.substr(0, pos);
    if (pipeCommand.size() == 0) {
      std::cerr << "myShell: syntax error near unexpected token `|'\n";
      return -1;
    }
    command = command.substr(pos + 1);
    command = parser.trim(command);
    pos = command.find("|");
    // read command and split into argv
    readOneCmd(pipeCommand);
  }
  // read the last command and split into argv
  readOneCmd(command);
  return 0;
}

/*
  needMoreInput

  This function detect whether there is odd '"' in the string or whether is end by '|'

  INPUT
  command: the command input by user

  OUTPUT
  bool: if have odd '"' or end by '|' return true, else,false
*/
bool needMoreInput(std::string command) {
  int count_quote = 0;
  for (char c : command) {
    if (c == '"')
      ++count_quote;
  }
  return (count_quote % 2 == 1) || command[command.find_last_not_of(' ')] == '|';
}

/*
  getline

  This function get a string

  INPUT
  tmp: where the string store

  OUTPUT
  bool: if meet eof,return false,else true;
*/
bool getline(std::string & tmp) {
  getline(std::cin, tmp);
  if (std::cin.eof()) {
    std::cout << "\n";
    return false;
  }
  return true;
}

/*
  getCommand

  This function will get the input from user,if user only input one",
  he should input more.

  INPUT
  command: where the string should be stored

  OUTPUT
  bool: if meet eof, return false, else true;

*/
bool getCommand(std::string & command) {
  std::string tmp;
  // read a line
  if (getline(tmp) == false)
    return false;
  command += tmp;
  // if the string contains odd '"',ask for more '"'
  while (needMoreInput(command)) {
    std::cout << ">";
    // read a line
    if (getline(tmp) == false)
      return false;
    command += tmp;
  }
  return true;
}

/*
  readCmds

  This function will handle with the input by user, read it and decide whether continue or not

  OUTPUT
  bool: if command is good to continue, return true

*/
bool Interface::readCmds() {
  // getCommand
  if (getCommand(command) == false) {
    return false;
  }
  // delete leading zero
  command = parser.trim(command, ' ');
  // judge whether user want to exit
  if (command == "exit") {
    return false;
  }
  return true;
}

/*
  interact

  This function print prompt,read command, handle with shell-set function, give arguments to execute.

  OUTPUT
  bool:if user input exit or meet oef, return false
 */
bool Interface::interact() {
  init();
  // print shell info
  prompt();
  // read user command
  if (readCmds() == false)
    return false;
  // store user-set vars
  storeUsrSetVar();
  // substitude var with value
  command = subVar4Value(command);
  if (detectPipes(command) == -1)
    clearAllCmd();
  envPath = parser.parseEnvPath();
  // execute shell funcs
  for (size_t i = 0; i < argvs.size(); i++) {
    if (execSysFunc(argvs[i])) {
      clearAllCmd();
    }
  }
  return true;
}

/*
  readOneCmd

  This function will read one command splited by '|'

  INPUT
  command: the whole line input by user

  OUTPUT
  0/-1

 */
int Interface::readOneCmd(std::string command) {
  if (command.size() == 0) {
    return 0;
  }
  // detect redirection in the given command line
  size_t pos = first_pos_of_rdi(command);
  std::string rdiStr = "";

  if (pos != std::string::npos) {
    rdiStr = command.substr(pos);
  }
  std::vector<int> dir;
  std::vector<std::string> filename;
  while (!rdiStr.empty())
    rdiStr = detectRdi(rdiStr, dir, filename);
  dirs.push_back(dir);
  filenames.push_back(filename);
  command = command.substr(0, pos);
  // after all shell word, parse the command line into arguments for executing
  std::vector<std::string> arg = parser.parseCmd(command);
  argvs.push_back(arg);
  return 0;
}

/*
  promt
  
  This function print shell info and init variables

 */
void Interface::prompt() {
  char * currentPath = get_current_dir_name();
  std::cout << "myShell:" << currentPath << " $";
  free(currentPath);
}

/*
  not_align_with_requir
 
  This function will judge whether given string meet the requirement of var
  
  INPUT
  s: a string contains var name
  
  OUTPUT
  bool: indicate var name legal or not
 
 */
bool not_align_with_requir(std::string s) {
  for (char c : s) {
    if (isalnum(c) || c == '_') {
    }
    else {
      return true;
    }
  }
  return false;
}

/*
  storeUsrSetVar
  
  This function will store user set vars, vars can only contain alpha, num and _

 */
void Interface::storeUsrSetVar() {
  // read the first var and its value
  std::string s = parser.readFirstWord(command, 1);
  size_t size_s = s.size();
  while (s.find("=") != std::string::npos) {
    // read the var name
    std::string key = parser.readFirstWord(s, 0, '=');
    // judge whether it meets requirement
    if (not_align_with_requir(key)) {
      return;
    }
    s = parser.delFirstWord(s, '=');
    // read the value
    std::string value = parser.readFirstWord(s, 1);
    // delete '\' inside
    value = parser.repSubStr(value, "\\");
    // store the var-value pair
    vars[key] = value;
    // get next var and its value
    command = command.substr(size_s);
    command = parser.trim(command, ' ');
    s = parser.readFirstWord(command, 1);
    size_s = s.size();
  }
}

/*
  myChdir
  
  This function will use the chdir function to change directory
  if wrong, handle with the error and return -1
  
  INPUT
  path:the path user want to go
  
  OUTPUT
  0/-1
  
*/
int myChdir(const char * path) {
  if (chdir(path) == -1) {
    perror("chdir");
    return -1;
  }
  return 0;
}

/*
  exec_cd

  This function will execute the cd command
  if wrong, handle with the error and return -1

  INPUT
  argv: command arguments

  OUTPUT
  0/-1: if fail return -1, else return 0
*/
int Interface::exec_cd(std::vector<std::string> argv) {
  // if given two or more arguments, then it is wrong
  if (argv.size() > 2) {
    std::cout << "-bash: cd: too many arguments\n";
    return -1;
  }
  // set the target directionay
  const char * targetDir =
      (argv.size() == 1) ? getenv("HOME") : (argv.size() == 2) ? argv[1].c_str() : nullptr;
  // change directory
  if (myChdir(targetDir) == -1) {
    std::cerr << "fail to change directory\n";
    return -1;
  }
  return 0;
}

/*
  exec_set

  This function will execute the set command
  if wrong, handle with the error and return -1

  INPUT
  argv: command arguments
  
  OUTPUT
  0/-1: if fail return -1, else return 0

*/
int Interface::exec_set(std::vector<std::string> argv) {
  // delete the word set
  command = parser.delFirstWord(command);
  // read the var
  std::string var = parser.readFirstWord(command);
  // judge whether it meets the requirement
  if (not_align_with_requir(var)) {
    std::cerr << var << ":var name should only include number, alpha and _\n";
    return -1;
  }
  // delete the name of var
  command = parser.delFirstWord(command);
  // read the value
  std::string value = command;
  // store in hashtable
  vars[var] = value;
  return 0;
}

/*
  exec_export

  This function will execute the export command
  if wrong, handle with the error and return -1

  INPUT
  argv: command arguments

  OUTPUT
  0/-1: if fail return -1, else return 0
*/
int Interface::exec_export(std::vector<std::string> argv) {
  // nothing to export, return success
  if (argv.size() == 1)
    return 0;
  // read the var
  std::string var = argv[1];
  if (not_align_with_requir(var)) {
    std::cerr << var << ":var name should only include number, alpha and _\n";
    return -1;
  }
  // if var exist in environment variables, revise it  if not, add to the list.

  // if the var exist in the table
  if (vars.count(var))
    return setenv(var.c_str(), vars[var].c_str(), true);
  std::cerr << var << ":no such variable\n";
  return -1;
}

/*
  exec_inc

  This function will execute the inc command
  if wrong, handle with the error and return -1

  INPUT
  argv: command arguments

  OUTPUT
  0/-1: if fail return -1, else return 0
*/
int Interface::exec_inc(std::vector<std::string> argv) {
  // nothing to inc, return 0
  if (argv.size() == 1)
    return 0;
  // read the var
  std::string var = argv[1];
  if (not_align_with_requir(var)) {
    std::cerr << var << ":var name should only include number, alpha and _\n";
    return -1;
  }
  // get the value based on var
  std::string value = vars[var];
  // if it is a number, +1,if not =1
  if (parser.isnum(value)) {
    value = parser.inc(value);
  }
  else {
    value = "1";
  }
  // store it again
  vars[var] = value;
  return 0;
}

/*
  getEnvPath
  
  OUTPUT
  envPath:the env path list
*/
std::vector<std::string> Interface::getEnvPath() {
  return envPath;
}

/*
  getArgv
  
  OUTPUT
  argv:the argv list
*/
std::vector<std::vector<std::string> > Interface::getArgvs() {
  return argvs;
}

/*
  getFn

  OUTPUT
  filename: the filename used for redirection

*/

std::vector<std::vector<std::string> > Interface::getFns() {
  return filenames;
}
/*
  getDir

  OUTPUT
  dir: the direction of the stream
 */

std::vector<std::vector<int> > Interface::getDirs() {
  return dirs;
}

/*
  constructor

  This function will store environment variable into vars

*/
Interface::Interface() {
  char ** envir = environ;
  while (*envir) {
    if (storeEnvVar(std::string(*envir)) == -1) {
      std::cerr << "Read Environment Variable fail\n";
    }
    envir++;
  }
}

/*
  storeEnvVar

  This function store environment variable in to shell vars

  INPUT
  s: the string contains environemtn variable

  OUTPUT
  0/-1
*/
int Interface::storeEnvVar(std::string s) {
  // read the var
  std::string var = parser.readFirstWord(s, 0, '=');
  // judge whether it meets the requirement
  if (not_align_with_requir(var)) {
    return -1;
  }
  // delete the name of var
  s = parser.delFirstWord(s, '=');
  // read the value
  std::string value = s;
  // store in hashtable
  vars[var] = value;
  return 0;
}
