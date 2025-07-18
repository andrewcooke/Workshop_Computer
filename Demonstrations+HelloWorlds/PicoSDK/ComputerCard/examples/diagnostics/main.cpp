
#include <algorithm>
#include <cmath>

#include "ComputerCard.h"


class Diagnostics : public ComputerCard {

private:

  constexpr static uint n_leds = 6;  // should be exposed by parent

  // i'm a sucker for flashing lights
  
  void column10levels(const bool up, const uint c, uint8_t v) {
    v = std::min(static_cast<uint8_t>(9), v);
    for (uint i = 0; i < n_leds / 2; i++) {
      uint led = up ? 4 + (c & 0x1) - 2 * i : (c & 0x1) + 2 * i;
      LedBrightness(led, static_cast<uint16_t>(std::min(static_cast<uint8_t>(3), v) << 10));
      v = v > 3 ? v - 3 : 0;
    }
  }
  
  void column10levels(const uint c, uint8_t v) {
    column10levels(true, c, v);
  }
  
  void columns12bits(uint16_t v) {
    v = v & 0x0fff;
    uint8_t v1 = v / 409;
    uint8_t v2 = (v % 409) / 41;
    column10levels(0, v1);
    column10levels(1, v2);
  }
  
  void columns12bits(int16_t v) {
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
    for (uint i = 0; i  < n_leds; i++) {
      LedBrightness(inv ? 5 - i : i, (i == v ? 4095 : (inv ? 1024 : 0)));
    }
  }

  void display3levels(uint8_t v) {
    for (uint i = 0; i < n_leds; i++) {
      LedBrightness(i, v == i / 2 ? 4095 : 0);
    }
  }

  // detect and display changes, or identify source of previous change

  constexpr static uint NONE = 999;
  uint prev_change = NONE;
  int32_t recent = 0;

  constexpr static uint noise_knobs = 4;
  constexpr static uint n_knobs = 3;
  uint32_t knobs[2][n_knobs] = {};
  constexpr static uint noise_switches = 0;
  constexpr static uint n_switches = 1;
  uint32_t switches[2][n_switches] = {};
  constexpr static uint noise_adcs = 7;
  constexpr static uint n_adcs = 4;
  uint32_t adcs[2][n_adcs] = {};
  constexpr static uint noise_pulses = 0;
  constexpr static uint n_pulses = 1;
  bool pulses[2][n_pulses] = {};

  constexpr static uint n_all = n_knobs + n_switches + n_adcs + n_pulses;

  uint32_t count = 0;
  constexpr static uint wtable_bits = 12;
  constexpr static uint wtable_size = 1 << wtable_bits;
  int16_t wtable[wtable_size] = {};

  void save_current() {
    for (uint i = 0; i < n_knobs; i++) {
      knobs[1][i] = knobs[0][i];
      // for some reason knob values are signed integers, but we
      // need to display unsigned so cast here
      knobs[0][i] = KnobVal(static_cast<Knob>(i)) >> noise_knobs;
    }
    switches[1][0] = switches[0][0];
    switches[0][0] = SwitchVal();
    for (uint i = 0; i < n_adcs; i++) {
      adcs[1][i] = adcs[0][i];
      adcs[0][i] = (i < 2 ? AudioIn(i) : CVIn(i - 2)) >> noise_adcs;
    }
    for (uint i = 0; i < n_pulses; i++) {
      pulses[1][i] = pulses[0][i];
      pulses[0][i] = PulseIn(i);
    }
  }

  bool changed(uint idx) {
    if (idx < n_knobs) return knobs[0][idx] != knobs[1][idx];
    idx -= n_knobs;
    if (idx < n_switches) return switches[0][idx] != switches[1][idx];
    idx -= n_switches;
    if (idx < n_adcs) return adcs[0][idx] != adcs[1][idx];
    idx -= n_adcs;
    if (idx < n_pulses) return pulses[0][idx] != pulses[1][idx];
    idx -= n_pulses;
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
    for (uint i = 0; i < n_leds; i++) LedBrightness(i, 1024);
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
    if (idx < n_switches) {
      LedOn(3);
      LedOn(5);
      return;
    }
    idx -= n_switches;
    if (idx < n_adcs) {
      LedOn(idx);  // swap audio l/r
      return;
    }
    idx -= n_adcs;
    if (idx < n_pulses) {
      LedOn(idx + 4);
      return;
    }
    idx -= n_pulses;
  }

  void display(uint idx) {
    if (idx < n_knobs) {
      columns12bits(static_cast<uint16_t>(KnobVal(static_cast<Knob>(idx))));
      return;
    }
    idx -= n_knobs;
    if (idx < n_switches) {
      uint led = 2 - static_cast<uint>(SwitchVal());
      for (uint i = 0; i < n_leds; i++) {
        if (i == 2 * led || i == 2 * led + 1) LedOn(i);
        else LedOff(i);
      }
      return;
    }
    idx -= n_switches;
    if (idx < n_adcs) {
      columns12bits(idx < 2 ? AudioIn(idx) : CVIn(idx - 2));
    }
    idx -= n_adcs;
    if (idx < n_pulses) {
      for (uint i = 0; i < n_leds; i++) {
        if (i == idx || i == idx + 4) LedOn(i);
        else LedOff(i);
      }
      return;
    }
    idx -= n_pulses;
    return;
  }

  bool pulse(uint n) {
    return (count & (1 << n)) && ! ((count - 1) & (1 << n));
  }

  void write_out() {
    for (uint i = 0; i < 4; i++) {
      uint idx = (count >> (i + 3)) & (wtable_size - 1);
      if (i < 2) AudioOut(i, wtable[idx]);
      else CVOut(i - 2, wtable[idx]);
    }
    PulseOut(0, pulse(11));;
    PulseOut(1, !pulse(12));
    count++;
  }

  int delay(uint prev_change) {
    if (prev_change == n_knobs) return 30000;  // switch
    if (prev_change == n_all - 1) return 100;  // pulse
    return 2000;  // default
  }

  virtual void ProcessSample() {
    write_out();
    save_current();
    if (prev_change != NONE && ((recent-- > 0) || changed(prev_change))) {
      if (changed(prev_change)) recent = delay(prev_change);
      display(prev_change);
    } else {
      uint current_change = next_change(prev_change);
      if (current_change == NONE && prev_change != NONE) identify(prev_change);
      if (current_change != NONE) {
        prev_change = current_change;
        recent = delay(prev_change);
      }
    }
  }

public:

  Diagnostics() {
    for (uint i = 0; i < wtable_size; i++) wtable[i] = static_cast<int16_t>(2047 * sin(2 * M_PI * i / wtable_size));
  }

};


int main()
{
	Diagnostics dg;
	dg.Run();
};

