#ifndef __INTERFACE_H__
#define __INTERFACE_H__
#include <unistd.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser.h"
/*
  class Interface

        attribute:

	        parser       used to revise strings
		command      the command inputted by user
		argvs        the parsed command
		envPath      the parsed environment path
		vars         user defined var-value pair
		dirs         directions of each command
		filenames    filenames related to direction
 user-used method:

	        readCmd      to read command and run shell funcs
		getEnvPath   return envPath
		getArgv      return argv
 */
class Interface
{
 private:
  Parser parser;
  std::string command;
  std::vector<std::string> envPath;
  std::vector<std::vector<int> > dirs;
  std::vector<std::vector<std::string> > argvs;
  std::vector<std::vector<std::string> > filenames;
  std::unordered_map<std::string, std::string> vars;
  void init();
  void prompt();
  bool readCmds();
  void clearAllCmd();
  void storeUsrSetVar();
  int storeEnvVar(std::string s);
  int readOneCmd(std::string command);
  int detectPipes(std::string command);
  int exec_cd(std::vector<std::string> argv);
  int exec_inc(std::vector<std::string> argv);
  int exec_set(std::vector<std::string> argv);
  int exec_export(std::vector<std::string> argv);
  bool execSysFunc(std::vector<std::string> argv);
  std::string readRdiFilename(std::string command, size_t pos);
  std::string detectRdi(std::string command,
                        std::vector<int> & dir,
                        std::vector<std::string> & filename);
  std::string subVar4Value(std::string S);

 public:
  Interface();
  bool interact();
  std::vector<std::vector<int> > getDirs();
  std::vector<std::vector<std::string> > getFns();
  std::vector<std::string> getEnvPath();
  std::vector<std::vector<std::string> > getArgvs();
};
#endif
