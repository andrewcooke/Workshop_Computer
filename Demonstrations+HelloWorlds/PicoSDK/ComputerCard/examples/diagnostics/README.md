
# Diagnostics

The idea here is to use all the ports to provide a way to check that
things are assembled correctly.  All inputs (sockets and knobs) change
the LEDs, so you can instantly see if the signal is being received.

When there is no signal the LEDs show where the last input was
detected.

## Outputs

The audio and CV outputs provide slowly varying sine waves.  The pulse
outputs provide positive (left) and negative (right) going pulses.

## Inputs

The audio and CV inputs drive the LEDs something like a level meter.
The left column of LEDs shows the overall signal amplitude.  The right
column shows the finer details (so varies more quickly and is only
useful at very low frequencies).

The pulse inputs drive the top left and right LEDs, respectively.

When an audio, CV or pulse signal is used and then disconnected the
LEDs show which input was last used (top left LED is top left input,
etc).

Adjusting knobs or the switch moves the LEDs in similar ways.

When a knob or switch is used and then left alone the LEDs show which
kob or switch was last used (top knob is top block of four LEDs, left
knob (X) is lower left bar, etc).

## Bad Connections

These are obvious because the expected output is not seen.

## Noisy Connections

These are more annoying because they generate signal even when you are
not doing anything.  Generally the source can be identified from the
LEDs (when the nois pauses the LEDs identify the source - see
descriptions above).


