
#include <algorithm>

#include "ComputerCard.h"


class Diagnostics : public ComputerCard {

public:

private:

  constexpr static uint numLeds = 6;  // should be exposed by parent

  // i'm a sucker for flashing lights
  
  void column10levels(const bool up, const uint c, uint8_t v) {
    v = std::min(static_cast<uint8_t>(9), v);
    for (uint i = 0; i < numLeds / 2; i++) {
      uint led = up ? 4 + (c & 0x1) - 2 * i : (c & 0x1) + 2 * i;
      LedBrightness(led, static_cast<uint8_t>(std::min(static_cast<uint8_t>(3), v) << 6));
      v = v > 3 ? v - 3 : 0;
    }
  }
  
  void column10levels(const uint c, uint8_t v) {
    column10levels(true, c, v);
  }
  
  void columns12bits(uint32_t v) {
    v = v & 0x0fff;
    uint8_t v1 = v / 409;
    uint8_t v2 = (v % 409) / 41;
    column10levels(0, v1);
    column10levels(1, v2);
  }
  
  void columns12bits(int32_t v) {
    if (v >= 0) {
      columns11bits(true, v);
    } else {
      columns11bits(false, -v);
    }
  }
  
  void columns11bits(bool up, uint16_t v) {
    v = v & 0x07ff;
    uint8_t v1 = v / 204;
    uint8_t v2 = (v % 204) / 21;
    column10levels(up, 0, v1);
    column10levels(up, 1, v2);
  }
  
  void display12levels(uint8_t v) {
    v = std::min(static_cast<uint8_t>(12), v);
    bool inv = v > 5;
    if (inv) v -= 5;
    for (uint i = 0; i  < numLeds; i++) {
      LedBrightness(inv ? 5 - i : i, (i == v ? 4095 : (inv ? 1024 : 0)));
    }
  }

  void display3levels(uint8_t v) {
    for (uint i = 0; i < numLeds; i++) {
      LedBrightness(i, v == i / 2 ? 4095 : 0);
    }
  }

  uint hold = 0;
  uint prev_main = 0;
  uint sel = -1;

  virtual void ProcessSample() {
    uint main = KnobVal(Main);
    if (prev_main != main) {
      prev_main = main;
      sel = main / 456;
      display12levels(sel);
      hold = 48000;
    } else if (hold > 0) {
      hold--;
    } else {
      switch(sel) {
        case 0:
          columns12bits(KnobVal(X));
          break;
        case 1:
          columns12bits(KnobVal(Y));
          break;
        case 3:
          display3levels(SwitchVal());
          break;
      }
    }
	}
};

int main()
{
	Diagnostics dg;
	dg.Run();
};

