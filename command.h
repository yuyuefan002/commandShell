#ifndef __COMMAND_H__
#define __COMMAND_H__
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "parser.h"

/*
  class Command
       attribute:
             parser     use for parsing strings
             dirs       list of redirections
             envPath    list of environment path
	     files4RDI  list of filenames related to redirections
             argvs      list of args

user-used method:

             execCmd         execute the command
             executable      judge whether the current command is executable or not
 */
class Command
{
 private:
  Parser parser;
  std::vector<std::string> envPath;
  std::vector<std::vector<int> > dirs;
  std::vector<std::vector<std::string> > argvs;
  std::vector<std::vector<std::string> > files4RDI;
  int execCmds();
  bool checkExecPath(std::string & path);
  int execCmd(char * argv[], char * envp[]);
  int waitChildren(std::vector<pid_t> children);
  int redirection(char * filename, int in_or_out);
  std::vector<pid_t> genChildProcess(int * pipefd);
  int childProcess(char ** argv,
                   char ** envp,
                   std::vector<int> & dir,
                   std::vector<std::string> & file4RDI,
                   int * pipefd,
                   size_t proc_index);

 public:
  void execCmd();
  bool executable();
  Command(std::vector<std::vector<std::string> > a,
          std::vector<std::string> e,
          std::vector<std::vector<int> > d,
          std::vector<std::vector<std::string> > f);
};
#endif
