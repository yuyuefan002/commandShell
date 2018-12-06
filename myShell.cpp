#include "command.h"
#include "interface.h"

// to debug multi-thread
// use"set follow-fork-mode-child"+main breakpoint
int main() {
  Interface interface;
  // read command
  while (interface.interact()) {
    Command cmd(
        interface.getArgvs(), interface.getEnvPath(), interface.getDirs(), interface.getFns());
    // judge wether command executable
    if (cmd.executable()) {
      // run command
      cmd.execCmd();
    }
  }

  return EXIT_SUCCESS;
}
