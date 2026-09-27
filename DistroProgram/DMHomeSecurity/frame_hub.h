// Takes pictures from the camera and shares each one with everyone watching.
// The camera only runs while at least one person is watching, and one picture
// is shared by all viewers, so extra viewers don't slow the camera down.
#pragma once
#include <Arduino.h>

namespace FrameHub {
  struct Frame {        // one JPEG picture
    uint8_t* buf;
    size_t   len;
    uint32_t seq;
    uint32_t ms;       // when it was taken
  };

  bool begin();
  void addDemand();     // "someone started watching"
  void dropDemand();    // "someone stopped watching"
  // Wait for a picture newer than afterSeq. Returns nullptr if none arrives in
  // time. Always call release() when you are done with the picture.
  const Frame* waitNext(uint32_t afterSeq, uint32_t timeoutMs);
  void release(const Frame* f);

  float    fps();       // actual frames per second
  uint32_t drops();     // pictures skipped (too big or memory busy)
  int      demand();
}
