#include "Context.h"
#include "HazeVersion.h"
#include "logc/log.h"
#include "server/HazeServer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>


void VersionMessage(void) { fprintf(stdout, "haze %s\n", HAZE_VERSION_STR); }

int TraitArgs(int argc, char **argv) {
  if (argc > 1) {
    const char *flag = argv[1];
    if (strlen(flag) == 1) {
      printf("error: too short flag.\n");
      return 0;
    }
    if (strcmp(flag, "-v") == 0 || strcmp(flag, "--version") == 0) {
      VersionMessage();
      return 0;
    } else {
      printf("unknown flag: '%s'.\n", flag);
      return 0;
    }
  }
  return 1;
}

int main(int argc, char **argv) {
  if (!TraitArgs(argc, argv)) {
    return 0;
  }
  Context* gi = ContextNew();

  log_info("Initializing Haze service (version %s)...", HAZE_VERSION_STR);
  log_info("Audio engine started successfully.");

  int port = InstanceRegistryGetLastPort(ContextGetPaths(gi));
  HazeServer *mainServer = NULL;

  while (1) {
    mainServer = HazeServerNew(NULL, port);

    if (!mainServer) {
      log_error("Failed to create Haze Server instance.");
      return 1;
    }

    int err = HazeServerStart(gi, mainServer);

    if (err == UV_EADDRINUSE || err == UV_EACCES) {
      log_warn("Port %d is unavailable (%s), trying port %d...",
               port, uv_strerror(err), port + 1);

      HazeServerFree(&mainServer);
      port++;
      continue;
    }

    if (err != 0) {
      log_error("Failed to start server: %s", uv_strerror(err));
      HazeServerFree(&mainServer);
      return 1;
    }

    log_info("Haze Server successfully started on %s:%d",
             HazeServerAddress(mainServer), HazeServerPort(mainServer));
    break; 
  }

  HazeServerSetupSignals(mainServer);

  HazeServerRun(mainServer);

  log_info("Shutting down and cleaning up resources...");


  HazeServerFree(&mainServer);
  log_info("cleaning context...");
  ContextFree(&gi);

  log_info("Haze service shut down gracefully.");
  return 0;
}
