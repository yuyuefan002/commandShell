#include "command.h"

/*
  createPipe

  This function will create the pipe for the subprocess

  INPUT
  num_pipe: This is the number of pipes which will be created. 
            It's the number of subprocess minus 1 
            since every two processes only need one pipe
 
  OUTPUT
  pipefd: This is a pointer pointing at the space for pipe
*/
int * createPipe(size_t num_pipe) {
  // only need argvs.size()-1 pipes, every two child process need one pipe
  int * pipefd = new int[num_pipe * 2];
  // init every pipe
  for (size_t i = 0; i < num_pipe; i++) {
    if (pipe(pipefd + 2 * i) == -1) {
      perror("pipe\n");
      exit(EXIT_FAILURE);
    }
  }
  return pipefd;
}

/*
  closePipe

  This function will close the pipe for the subprocess

  INPUT
  pipefd: the pointer pointing at the space of pipes
  num_pipe: the number of pipes, pointer cannot know the number of pipes

  OUTPUT
  0/-1: if fail return -1, else ,0
*/
int closePipe(int * pipefd, size_t num_pipe) {
  // close every pipe
  for (size_t i = 0; i < num_pipe * 2; i++) {
    if (close(pipefd[i]) == -1) {
      perror("error\n");
      return -1;
    }
  }
  return 0;
}

/*
  pipe

  This function will updata every subprocess's input and output to the correct stream

  INPUT
  pipefd: the pointer pointing at the space of pipes
  proc_index: current process index
  num_pipe: the total number of pipes

  OUTPUT
  0/-1: if fail return -1, else 0
 */
int pipe(int * pipefd, size_t proc_index, size_t num_pipe) {
  // if not the first function, since no in pipe connect with the first function
  if (proc_index != 0) {
    // stdin=0, read from pipe (n-1)*2 read from previous process
    if (dup2(pipefd[(proc_index - 1) * 2], 0) == -1) {
      perror("dup2");
      return -1;
    }
  }
  // if not the last function, since no out pipe connect with the last function
  if (proc_index != num_pipe) {
    // stdout=1,write to pipe 2n+1,current write,next read
    if (dup2(pipefd[(proc_index * 2) + 1], 1) == -1) {
      perror("dup2");
      return -1;
    }
  }
  // cloase all the pipes
  closePipe(pipefd, num_pipe);
  return 0;
}

/*
  genChildProcess

  This function will generate every child process

  INPUT
  pipefd: the pointer pointing at the pipe space

  OUTPUT
  children: the children pid array
 */
std::vector<pid_t> Command::genChildProcess(int * pipefd) {
  std::vector<pid_t> children;
  for (size_t i = 0; i < argvs.size(); i++) {
    // create argv pointer
    char ** argv_ptr = parser.strList2charStarList(argvs[i]);
    pid_t cpid;
    // children get 0, parent get children' s pid
    if ((cpid = fork()) == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    else if (cpid == 0) {
      // run child process
      childProcess(argv_ptr, environ, dirs[i], files4RDI[i], pipefd, i);
    }
    // free argv pointer
    delete[] argv_ptr;
    // add children pid into vector, so that parent can wait these pid
    children.push_back(cpid);
  }
  return children;
}

/*
  execCmds

  This function will fork() the current thread as the child thread
            execve() another program on the child thread
	    waitpid() until the child thread return

   OUTPUT
  -1/0:indicate success or not

 */
int Command::execCmds() {
  std::vector<pid_t> children;

  // create corresponding pipes
  int * pipefd = createPipe(argvs.size() - 1);
  // generate every child process
  children = genChildProcess(pipefd);
  // parent process will not use these pipes, so close
  if (closePipe(pipefd, argvs.size() - 1) == -1) {
    std::cerr << "closePipe error\n";
    return -1;
  }
  // parent procress, wait until all children return
  if (waitChildren(children) == -1) {
    std::cerr << "fail to wait for children processes.\n";
    return -1;
  }
  // after execution, delete those space
  delete[] pipefd;

  return 0;
}

/*
  childProcess

  This function will execute the program in child process

  INPUT
  argv: arguments of upcoming program
  envp: environment variables of upcoming program
  dir: redirection
  file4RDI: redirection filename
  pipefd: the pointer pointing at the pipe space
  proc_index: the index of current child process

  OUTPUT
  0/-1: indicating success or not

*/
int Command::childProcess(char ** argv,
                          char ** envp,
                          std::vector<int> & dir,
                          std::vector<std::string> & file4RDI,
                          int * pipefd,
                          size_t proc_index) {
  // if there is a redirection

  // update the pipe stream
  pipe(pipefd, proc_index, argvs.size() - 1);
  for (size_t i = 0; i < dir.size(); i++) {
    if (dir[i] != -1) {
      // update the target stream
      redirection((char *)file4RDI[i].c_str(), dir[i]);
    }
  }
  // execute the program
  execve(argv[0], argv, envp);
  _exit(EXIT_SUCCESS);
  return 0;
}

/*
  waitChildren

  This function will let the parent process wait until all child process exit

  INPUT
  children: the pid of children process

  OUTPUT
  -1/0
 */
int Command::waitChildren(std::vector<pid_t> children) {
  int wstatus;
  // wait every child reuturn
  for (size_t i = 0; i < children.size(); i++) {
    // parent stop here until coresponding process finish
    if (waitpid(children[i], &wstatus, /*WUNTRACED | WCONTINUED*/ 0) == -1) {
      perror("waitpid");
      return -1;
    }
    if (WIFEXITED(wstatus)) {
      std::cout << "Program exited with status " << WEXITSTATUS(wstatus) << std::endl;
    }
    else if (WIFSIGNALED(wstatus)) {
      std::cout << "Program was killed by signal " << WTERMSIG(wstatus) << std::endl;
    }
  }
  return 0;
}

/*
  isRegFile

  This function willcheck out whether the file is excutable

  INPUT
  file: the given path of the program

  OUTPUT
  true/false: if the path is executable,return true, else, false

*/
bool isRegFile(std::string file) {
  struct stat sb;
  // read the file info
  if (stat(file.c_str(), &sb) == 0) {
    // check whether the file is regular or not.Yes? return true.No? false
    return S_ISREG(sb.st_mode);
  }
  //fail to read
  return false;
}

/*
  updExecpath

  This function will set the correct path for the callee program

  INPUT
  path:the path of upcoming program

  OUTPUT
  bool: update sucess or not

*/
bool updExecPath(std::string & path, std::vector<std::string> envPath) {
  // if exist /, it is not a global variable
  if (path.find('/') != std::string::npos) {
    // check the program exist or not
    return isRegFile(path);
  }
  // search environment path to find whether command exist or not
  for (std::string s : envPath) {
    std::string newPath;
    newPath = s + "/" + path;
    if (isRegFile(newPath)) {
      // it exist, generate the correct path
      path = newPath;
      return true;
    }
  }
  // not a valid command, return false
  return false;
}

/*
  updPathErr

  print error info when command cannot be found

  INPUT
  path: the invalid command

 */
void updPathErr(std::string path) {
  std::cerr << "Command " << path << " not found\n";
}

/*
  checkExecPath
  
  This function will return true if given path is valid.

  INPUT
  path:the path for checking

  OUTPUT
  bool:function success or not

*/
bool Command::checkExecPath(std::string & path) {
  // set the correct path for program
  if (updExecPath(path, envPath) == false) {
    // if fail, print error info
    updPathErr(path);
    return false;
  }
  return true;
}

/*
  checkRdiFile

  This function will check whether the given filename is good for redirection

  INPUT
  file4RDI: the file path for redirection

  OUTPUT
  bool: if bad for redirection,return false, else true
 */
bool checkRdiFile(std::string file4RDI) {
  // if given file is bad
  if (isRegFile(file4RDI) == false) {
    std::cerr << "myShell: " << file4RDI << ": No such file or directory\n";
    return false;
  }
  return true;
}

/*
  checkRdiFileGood

  This function checks whether the file related to RDI is good or not

  INPUT
  dir:direction of redirection
  file: the file related to the direction

  OUTPUT
  bool: if good return true, else,false
*/
bool checkRdiFileGood(int dir, std::string file) {
  if (dir == 0 || dir == 2) {
    if (file.size() == 0) {
      std::cerr << "myShell: syntax error near unexpected token `>'\n";
      return false;
    }
  }
  else if (dir == 1) {
    if (file.size() == 0) {
      std::cerr << "myShell: syntax error near unexpected token `<'\n";
    }
    else if (checkRdiFile(file) == false)
      return false;
  }
  return true;
}

/*
  executable

  This function indicate whether the command is executable or not

  OUTPUT
  bool: executable return true, else return false

*/
bool Command::executable() {
  // no command, it means executed shell function
  if (argvs.size() == 0)
    return false;
  // check whether each program existed
  for (size_t i = 0; i < argvs.size(); i++) {
    if (checkExecPath(argvs[i][0]) == false)
      return false;
    // check whether rdi filename is correct
    for (size_t j = 0; j < dirs[i].size(); j++) {
      if (checkRdiFileGood(dirs[i][j], files4RDI[i][j]) == false)
        return false;
    }
  }
  return true;
}

/*
type:constructor
description: read args list,envp list,set the good.
             if user want exit, good is false.
 */
Command::Command(std::vector<std::vector<std::string> > a,
                 std::vector<std::string> e,
                 std::vector<std::vector<int> > d,
                 std::vector<std::vector<std::string> > f) :
    envPath(e),
    dirs(d),
    argvs(a),
    files4RDI(f) {}

/*
  execCmd

  run the command

*/
void Command::execCmd() {
  if (execCmds() == -1) {
    std::cerr << "fail to run program!\n";
  }
}

/*
  rdiIn

  This function set the in stream to the given file

  INPUT
  filename: the file user want stream going into

  OUTPUT
  0/-1

*/
int rdiIn(char * filename) {
  int in;
  in = open(filename, O_RDONLY);
  if (dup2(in, 0) == -1) {
    perror("dup2");
    return -1;
  }
  close(in);
  return 0;
}

/*
  rdiOut

  This function set the out stream to the given file

  INPUT
  filename: the file user want stream going into

  OUTPUT
  0/-1

 */
int rdiOut(char * filename) {
  int out;
  out = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
  if (dup2(out, 1) == -1) {
    perror("dup2");
    return -1;
  }
  close(out);
  return 0;
}

/*
  rdiErr

  This function set the err stream to the given file

  INPUT
  filename: the file user want stream going into

  OUTPUT
  0/-1

 */
int rdiErr(char * filename) {
  int err;
  err = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
  if (dup2(err, 2) == -1) {
    perror("dup2");
    return -1;
  }
  close(err);
  return 0;
}

/*
  redirection

  This function set the currect stream target according to direction

  INPUT
  in_or_out: the direction need to be revised

  OUTPUT
  0/-1:indicating success or not

 */
int Command::redirection(char * filename, int in_or_out) {
  if (in_or_out == 1)
    return rdiIn(filename);
  else if (in_or_out == 0)
    return rdiOut(filename);
  else if (in_or_out == 2)
    return rdiErr(filename);
  return 0;
}
