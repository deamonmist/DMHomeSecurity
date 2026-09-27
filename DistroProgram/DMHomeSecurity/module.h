// Each feature of the program is a "module" with a name, a start function,
// and an optional function that runs over and over.
//
// To add your own feature:
//   1. Make a new .cpp file that defines  const Module kModMyFeature = {...};
//   2. Add  EXTERN_MODULE(kModMyFeature);  to the list below.
//   3. Add  &kModMyFeature,  to the MODULES list in DMHomeSecurity.ino.
#pragma once
#include <Arduino.h>

struct Module {
  const char* name;
  bool (*begin)();      // runs once at start-up; return false if it failed
  void (*loop)();       // runs repeatedly (can be nullptr)
  bool required;        // true = restart the board if this part fails
};

#define EXTERN_MODULE(x) extern const Module x

EXTERN_MODULE(kModStatusLed);
EXTERN_MODULE(kModNet);
EXTERN_MODULE(kModCamera);
EXTERN_MODULE(kModHttp);
EXTERN_MODULE(kModCameraApi);
EXTERN_MODULE(kModRegistry);   // master only
EXTERN_MODULE(kModWebUi);      // master only
EXTERN_MODULE(kModSlavePage);  // slave only
EXTERN_MODULE(kModHeartbeat);  // slave only
EXTERN_MODULE(kModOta);
