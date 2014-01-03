#include "StrPrintf.h"
#include "Logging.h"
#include "TimeVal.h"
#include "TimerEvent.h"
#include "Cmd.h"
#include "CmdServo.h"

const int led = LED_BUILTIN;

class HeartBeat : public TimerEvent
{
public:
//  HeartBeat() : TimerEvent(TimerEvent::REPEAT, MilliSeconds(100)), _count(0) {}
  HeartBeat() : TimerEvent("HeartBeat"), _count(0) {}
  virtual void onEvent(const TimeVal &when)
  {
    if ((_count & 0xC) == 0) {
      digitalWrite(led, !(_count & 1));
    }
    _count = (_count + 1) % 10;
  }

private:
  int _count;
} gHeartBeat;

class Foo : public TimerEvent
{
public:
  Foo() : TimerEvent("Foo"), _count(0) {}
  virtual void onEvent(const TimeVal &when)
  {
    Serial.print((char)('0' + _count));
    if (_count == 9) {
      Serial.write("\r\n");
    }
    _count = (_count + 1) % 10;
  }
private:
  int _count;
} gFoo;

void aw(int argc, char **argv)
{
  if (argc != 3) {
    Cmd::Printf("Expecting 2 arguments. Found %d\n", argc - 1);
    return;
  }

  uint8_t pin;
  if (!Cmd::ParseInt("pin", argv[1], &pin)) {
    return;
  }
  uint32_t val;
  if (!Cmd::ParseInt("val", argv[2], &val)) {
    return;
  }
  Cmd::Printf("analogWrite(%u, %u)\n", pin, val);
  analogWrite(pin, val);
}

void awf(int argc, char **argv)
{
  if (argc != 3) {
    Cmd::Printf("Expecting 2 arguments. Found %d\n", argc - 1);
    return;
  }

  uint8_t pin;
  if (!Cmd::ParseInt("pin", argv[1], &pin)) {
    return;
  }
  uint32_t freq;
  if (!Cmd::ParseInt("val", argv[2], &freq)) {
    return;
  }
  Cmd::Printf("analogWriteFrequency(%u, %u)\n", pin, freq);
  analogWriteFrequency(pin, freq);
}

void awr(int argc, char **argv)
{
  if (argc != 2) {
    Cmd::Printf("Expecting 1 argument. Found %d\n", argc - 1);
    return;
  }
  uint32_t resolution;
  if (!Cmd::ParseInt("resolution", argv[1], &resolution)) {
    return;
  }
  Cmd::Printf("analogWriteResolution(%u)\n", resolution);
  analogWriteResolution(resolution);
}

const Cmd::Entry gCmdEntry[] =
{
  { Cmd::Args,  "args",  "",           "Displays arguments passed on command line" },
  { aw,         "aw",    "pin val",    "Calls analogWrite(pin, val)" },
  { awf,        "awf",   "pin freq",   "Calls analogWriteFrequeny(pin, frequency)" },
  { awr,        "awr",   "resoltion",  "Calls analogWriteResolution(resolution)" },
  { Cmd::Help,  "help",  "[command]",  "Displays help for a given command (or all commands)" },
  { CmdServo,   "servo", "pin sub-command", "Servo class commands", gCmdServoEntry },
  { nullptr, nullptr, nullptr, nullptr }
};

Cmd gCmd(gCmdEntry);

void setup() {
  Log.Init(Logging::DEBUG);
  while (!Serial) ;
  Log.Warning("Rover Started");

  gHeartBeat.Repeat(MilliSeconds(100));
  //gFoo.Repeat(MilliSeconds(1000));

  pinMode(led, OUTPUT);

  gTimerQueue.Dump();


  // For Teensy 3.1 F_CPU == 96000000
  //                F_BUS == 48000000
  //                F_MEM == 24000000

  // PWM freq 50-60 Hz
  // @ 50 Hz = 20 msec = 20,000 usec

  // analogWriteResolution(16)  - global
  // analogWriteFrequency(pin, freq)
  // analogWrite(pin, dutyCycle)

  // void analogWrite(uint8_t pin, int val);
  // void analogWriteResolution(uint32_t bits)
  // void analogWriteFrequency(uint8_t pin, uint32_t frequency);

  // @ 48 MHz, prescalar of 4: 48000000 / 65536 / 16 = 45.77 Hz (46)
  // So analogWriteFrequency(pin, 46)
  // freq = 46 mod = 65216

  // If I pick freq of 50 Hz (period = 20 msec)
  // mod == 60000
  //
  // 1/20 * 65536 = 3276
  // 2/20 * 65536 = 6553


  // pins 3, 4               are FTM1
  // pins 5, 6, 9, 10, 20-23 are FTM0
  //
  // pins 16 & 17            are FTM1 (but are missing from analogWriteFrequency?)
  // pins 25 & 32            are FTM2 (but are missing from analogWriteFrequency?)

  // Steering is on pins 20 & 21
  // Pins 9 is throttle

}

void loop() {
  gTimerQueue.Run();
  gEventQueue.Run();
  gCmd.Run();
}
