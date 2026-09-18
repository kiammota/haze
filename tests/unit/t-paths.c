#include "fs/Paths.h"
#include "macros.h"

static int TestPathsNew(void) {
  TEST_LOG("PathsNew");

  Paths *paths = PathsNew();

  TEST_ASSERT(paths != NULL);

  TEST_PASS();
}

static int TestPathsGet(void) {
  TEST_LOG("PathsGet");

  Paths *paths = PathsNew();

  TEST_ASSERT(paths != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_CACHE) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_CONFIG) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_DATA) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_PROJECTS) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_SAMPLES) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_THEMES) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_PLUGINS) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_LOGS) != NULL);
  TEST_ASSERT(PathsGet(paths, PATHS_INSTANCES) != NULL);

  TEST_PASS();
}

int main(void) {
  TEST_LOG("Running Paths tests");

  int failed = 0;

  failed += TestPathsNew() != 0;
  failed += TestPathsGet() != 0;

  if (failed == 0) {
    TEST_LOG("All Paths tests passed");
    return 0;
  }

  TEST_LOG("Some Paths tests failed");
  return 1;
}
