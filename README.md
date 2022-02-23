## What is this
This is the Arduino code to make  festival totems that can find each other without needing cell
data service (since that tends to break down at large music festivals). All you need is GPS
reception and not-overly-crowded 900mhz radio spectrum.

It's currently a work in progress. The end goal is that the totems will glow their LEDs in the
direction of other totems at festivals, with some kind of hardware switch to toggle between linked
totems.

## Hardware

You'll need a bunch:

* Adafruit Feather M0 LoRa (900mhz version)
* A 900mhz band antenna
* Adafruit Ultimate GPS module, v3
* A GPS antenna
* Adafruit triple-axis magnetometer LIS3MDL
* Some lights? A toggle switch? TBD

## Setup

The first time you clone the repo, and every time you modify the radio protocol, you'll need to
rebuild the serialization/deserialization code. Run:

```
ruby ./generate_serde.rb
```

You'll also need to make sure your Arduino IDE (or whatever Arduino toolchain you're using) is set
up for the Adafruit Feather LoRa boards we use.
[Follow the instructions here to add the boards.](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/setup)

Finally, you'll need to install the radio library we use. Since C++ is the worst, of course you'll
need to do this manually.
[Follow the instructions here.](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/using-the-rfm-9x-radio)

Once you're all set up, this repo should build/upload/etc with ordinary Arduino tools.