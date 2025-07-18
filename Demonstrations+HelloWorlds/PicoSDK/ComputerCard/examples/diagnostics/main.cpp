
#include <algorithm>

#include "ComputerCard.h"


class Diagnostics : public ComputerCard {

public:

private:

  constexpr static uint num_leds = 6;  // should be exposed by parent

  // i'm a sucker for flashing lights
  
  void column10levels(const bool up, const uint c, uint8_t v) {
    v = std::min(static_cast<uint8_t>(9), v);
    for (uint i = 0; i < num_leds / 2; i++) {
      uint led = up ? 4 + (c & 0x1) - 2 * i : (c & 0x1) + 2 * i;
      LedBrightness(led, static_cast<uint16_t>(std::min(static_cast<uint8_t>(3), v) << 10));
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
    for (uint i = 0; i  < num_leds; i++) {
      LedBrightness(inv ? 5 - i : i, (i == v ? 4095 : (inv ? 1024 : 0)));
    }
  }

  void display3levels(uint8_t v) {
    for (uint i = 0; i < num_leds; i++) {
      LedBrightness(i, v == i / 2 ? 4095 : 0);
    }
  }

  // detect and display changes, or identify source of previous change

  constexpr static uint NONE = 999;
  uint prev_change = NONE;
  int32_t recent = 0;
  constexpr static uint NOISE_KNOBS = 3;
  constexpr static uint n_knobs = 3;
  uint32_t knobs[2][n_knobs] = {};
  constexpr static uint NOISE_ADC = 6;
  constexpr static uint n_adc = 4;
  uint32_t adcs[2][n_adc] = {};
  constexpr static uint n_all = n_knobs + n_adc;

  void save_current() {
    for (uint i = 0; i < n_knobs; i++) {
      knobs[1][i] = knobs[0][i];
      knobs[0][i] = KnobVal(static_cast<Knob>(i)) >> NOISE_KNOBS;
    }
    for (uint i = 0; i < n_adc; i++) {
      adcs[1][i] = adcs[0][i];
      adcs[0][i] = (i < 2 ? AudioIn(i) : CVIn(i-2)) >> NOISE_ADC;
    }
  }

  bool changed(uint idx) {
    if (idx < n_knobs) return knobs[0][idx] != knobs[1][idx];
    idx -= n_knobs;
    if (idx < n_adc) return adcs[0][idx] != adcs[1][idx];
    idx -= n_adc;
    return false;
  }

  uint next_change(uint old_idx) {
    uint new_idx = old_idx == NONE ? 0 : old_idx + 1;
    for (uint i = 0; i < n_all; i++) {
      if (new_idx == n_all) new_idx = 0;
      if (changed(new_idx)) return new_idx;
      new_idx++;
    }
    return NONE;
  }

  void identify(uint idx) {
    for (uint i = 0; i < num_leds; i++) LedBrightness(i, 1024);
    if (idx < n_knobs) {
      switch(idx) {
      case static_cast<uint>(Main):
        for (uint i = 0; i < 4; i++) LedOn(i);
        return;
      case static_cast<uint>(X):
        LedOn(2);
        LedOn(4);
        return;
      case static_cast<uint>(Y):
        for (uint i = 2; i < 6; i++) LedOn(i);
        return;
      }
    }
    idx -= n_knobs;
    if (idx < n_adc) {
      LedOn(idx);  // swap audio l/r
      return;
    }
  }

  void display(uint idx) {
    if (idx < n_knobs) {
      columns12bits(KnobVal(static_cast<Knob>(idx)));
      return;
    }
    idx -= n_knobs;
    if (idx < n_adc) {
      columns12bits(static_cast<int32_t>(idx < 2 ? AudioIn(idx) : CVIn(idx - 2)));
    }
    return;
  }

  virtual void ProcessSample() {
    save_current();
    if (prev_change != NONE && ((recent-- > 0) || changed(prev_change))) {
      if (changed(prev_change)) recent = 1000;
      display(prev_change);
    } else {
      uint current_change = next_change(prev_change);
      if (current_change == NONE && prev_change != NONE) identify(prev_change);
      if (current_change != NONE) prev_change = current_change;
    }
  }

};


int main()
{
	Diagnostics dg;
	dg.Run();
};

