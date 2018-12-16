////////////////////////////////////////////////////////////////////
//                                                                //
// ORIGINAL CODE 12 Dec 2011 Mike Hord, SparkFun Electronics      //
//                                                                //
// LIBRARY Created by Adam Meyer (@ameyer) of bildr 18 Aug 2012   //
//        Modified by Scott Lahteine (@thinkyhead) 6 Mar 2018     //
//        Released as MIT license                                 //
//                                                                //
////////////////////////////////////////////////////////////////////

#include "L6470.h"

#include <Arduino.h>
#include <SPI.h>

L6470::L6470(const int SSPin) {
  _SSPin = SSPin;
  // Serial.begin(9600);
}

// Generic init function to set up communication with the dSPIN chip.
void L6470::init() {

  pinMode(_SSPin, OUTPUT);
  digitalWrite(_SSPin, HIGH);

  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);
  pinMode(BUSYN, INPUT);
  pinMode(RESET, OUTPUT);

  // reset the dSPIN chip. This could also be accomplished by
  //  calling the "L6470::ResetDev()" function after SPI is initialized.
  digitalWrite(RESET, HIGH);
  delay(10);
  digitalWrite(RESET, LOW);
  delay(10);
  digitalWrite(RESET, HIGH);
  delay(10);

  // initialize SPI for the dSPIN chip's needs:
  //  most significant bit first,
  //  SPI clock not to exceed 5MHz,
  //  SPI_MODE3 (clock idle high, latch data on rising edge of clock)
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16); // or 2, 8, 16, 32, 64
  SPI.setDataMode(SPI_MODE3);

  // First things first: let's check communications. The L6470_CONFIG register should
  //  power up to 0x2E88, so we can use that to check the communications.
  //Serial.println(GetParam(L6470_CONFIG) == 0x2E88 ? "good to go" : "Comm issue");

  // First, let's set the step mode register:
  //   - SYNC_EN controls whether the BUSY/SYNC pin reflects the step
  //     frequency or the BUSY status of the chip. We want it to be the BUSY
  //     status.
  //   - STEP_SEL_x is the microstepping rate- we'll go full step.
  //   - SYNC_SEL_x is the ratio of (micro)steps to toggles on the
  //     BUSY/SYNC pin (when that pin is used for SYNC). Make it 1:1, despite
  //     not using that pin.
  //SetParam(L6470_STEP_MODE, !SYNC_EN | STEP_SEL_1 | SYNC_SEL_1);

  SetParam(L6470_KVAL_RUN, 255);
  SetParam(L6470_KVAL_ACC, 255);
  SetParam(L6470_KVAL_DEC, 255);

  // Set up the L6470_CONFIG register as follows:
  //  PWM frequency divisor = 1
  //  PWM frequency multiplier = 2 (62.5kHz PWM frequency)
  //  Slew rate is 290V/us
  //  Do NOT shut down bridges on overcurrent
  //  Disable motor voltage compensation
  //  Hard stop on switch low
  //  16MHz internal oscillator, nothing on output
  SetParam(L6470_CONFIG, CONFIG_PWM_DIV_1 | CONFIG_PWM_MUL_2 | CONFIG_SR_290V_us| CONFIG_OC_SD_DISABLE | CONFIG_VS_COMP_DISABLE | CONFIG_SW_HARD_STOP | CONFIG_INT_16MHZ);
  // Configure the dSPIN_RUN KVAL. This defines the duty cycle of the PWM of the bridges
  //  during running. 0xFF means that they are essentially NOT PWMed during run; this
  //  MAY result in more power being dissipated than you actually need for the task.
  //  Setting this value too low may result in failure to turn.
  //  There are L6470_ACC, L6470_DEC, and HOLD KVAL registers as well; you may need to play with
  //  those values to get acceptable performance for a given application.
  SetParam(L6470_KVAL_RUN, 0xFF);
  // Calling GetStatus() clears the UVLO bit in the status register, which is set by
  //  default on power-up. The driver may not run without that bit cleared by this
  //  read operation.
  getStatus();

  hardStop(); //engage motors
}

boolean L6470::isBusy() { return !(getStatus() & 0x0002); }

void L6470::setMicroSteps(int microSteps) {
  byte stepVal;
  for (stepVal = 0; stepVal < 8; stepVal++) {
    if (microSteps == 1) break;
    microSteps >>= 1;
  }
  SetParam(L6470_STEP_MODE, !SYNC_EN | stepVal | SYNC_SEL_1);
}

// Configure the L6470_FS_SPD register- this is the speed at which the driver ceases
//  microstepping and goes to full stepping. FSCalc() converts a value in steps/s
//  to a value suitable for this register; to disable full-step switching, you
//  can pass 0x3FF to this register.
void L6470::setThresholdSpeed(const float thresholdSpeed) {
  SetParam(L6470_FS_SPD, thresholdSpeed ? FSCalc(thresholdSpeed) : 0x3FF);
}

void L6470::setCurrent(const int current) { (void)(current); }

// Configure the L6470_MAX_SPEED register- this is the maximum number of (micro)steps per
//  second allowed. You'll want to mess around with your desired application to see
//  how far you can push it before the motor starts to slip. The ACTUAL parameter
//  passed to this function is in steps/tick; MaxSpdCalc() will convert a number of
//  steps/s into an appropriate value for this function. Note that for any move or
//  goto type function where no speed is specified, this value will be used.
void L6470::setMaxSpeed(const int speed) { SetParam(L6470_MAX_SPEED, MaxSpdCalc(speed)); }

// Configure the L6470_MAX_SPEED register- this is the maximum number of (micro)steps per
//  second allowed. You'll want to mess around with your desired application to see
//  how far you can push it before the motor starts to slip. The ACTUAL parameter
//  passed to this function is in steps/tick; MaxSpdCalc() will convert a number of
//  steps/s into an appropriate value for this function. Note that for any move or
//  goto type function where no speed is specified, this value will be used.
void L6470::setMinSpeed(const int speed) { SetParam(L6470_MIN_SPEED, MinSpdCalc(speed)); }

// Configure the acceleration rate, in steps/tick/tick. There is also a L6470_DEC register;
//  both of them have a function (AccCalc() and DecCalc() respectively) that convert
//  from steps/s/s into the appropriate value for the register. Writing L6470_ACC to 0xfff
//  sets the acceleration and deceleration to 'infinite' (or as near as the driver can
//  manage). If L6470_ACC is set to 0xFFF, L6470_DEC is ignored. To get infinite deceleration
//  without infinite acceleration, only hard stop will work.
void L6470::setAcc(const float acceleration) { SetParam(L6470_ACC, AccCalc(acceleration)); }

void L6470::setDec(const float deceleration) { SetParam(L6470_DEC, DecCalc(deceleration)); }

long L6470::getPos() { return convert(GetParam(L6470_ABS_POS)); }

// L6470_SPEED
//  The L6470_SPEED register contains the current motor speed, expressed in step/tick (format unsigned fixed point 0.28).
//  In order to convert the L6470_SPEED value in step/s the following formula can be used:
//  Equation 4
//  where L6470_SPEED is the integer number stored into the register and tick is 250 ns.
//  The available range is from 0 to 15625 step/s with a resolution of 0.015 step/s.
//  Note: The range effectively available to the user is limited by the L6470_MAX_SPEED parameter.
float L6470::getSpeed() {
  return (float)GetParam(L6470_SPEED);
  //return (float) speed * pow(8, -22);
  //return FSCalc(speed); NEEDS FIX
}

// Configure the overcurrent detection threshold.
void L6470::setOverCurrent(const unsigned int ma_current) {
  const byte OCValue = (byte)floor(ma_current / 375);
  SetParam(L6470_OCD_TH, OCValue < 0x0F ? OCValue : 0x0F);
}

void L6470::setStallCurrent(const float ma_current) {
  const byte STHValue = (byte)floor(ma_current / 31.25);
  SetParam(L6470_STALL_TH, STHValue < 0x80 ? STHValue : 0x80);
}

// Enable or disable the low-speed optimization option. If enabling,
//  the other 12 bits of the register will be automatically zero.
//  When disabling, the value will have to be explicitly written by
//  the user with a SetParam() call. See the datasheet for further
//  information about low-speed optimization.
void L6470::SetLowSpeedOpt(const boolean enable) {
  Xfer(dSPIN_SET_PARAM | L6470_MIN_SPEED);
  Param(enable ? 0x1000 : 0, 13);
}

// dSPIN_RUN sets the motor spinning in a direction (defined by the constants
//  dSPIN_FWD and dSPIN_REV). Maximum speed and minimum speed are defined
//  by the L6470_MAX_SPEED and L6470_MIN_SPEED registers; exceeding the L6470_FS_SPD value
//  will switch the device into full-step mode.
// The SpdCalc() function is provided to convert steps/s values into
//  appropriate integer values for this function.
void L6470::run(const byte dir, const float spd) {
  unsigned long speedVal = SpdCalc(spd);
  Xfer(dSPIN_RUN | dir);
  if (speedVal > 0xFFFFF) speedVal = 0xFFFFF;
  Xfer((byte)(speedVal >> 16));
  Xfer((byte)(speedVal >> 8));
  Xfer((byte)(speedVal));
}

// dSPIN_STEP_CLOCK puts the device in external step clocking mode. When active,
//  pin 25, STCK, becomes the step clock for the device, and steps it in
//  the direction (set by the dSPIN_FWD and dSPIN_REV constants) imposed by the call
//  of this function. Motion commands (dSPIN_RUN, dSPIN_MOVE, etc) will cause the device
//  to exit step clocking mode.
void L6470::Step_Clock(const byte dir) {
  Xfer(dSPIN_STEP_CLOCK | dir);
}

// dSPIN_MOVE will send the motor n_step steps (size based on step mode) in the
//  direction imposed by dir (dSPIN_FWD or dSPIN_REV constants may be used). The motor
//  will accelerate according the acceleration and deceleration curves, and
//  will run at L6470_MAX_SPEED. Stepping mode will adhere to L6470_FS_SPD value, as well.
void L6470::move(const long n_step) {
  const byte dir = n_step >= 0 ? dSPIN_FWD : dSPIN_REV;
  Xfer(dSPIN_MOVE | dir);         // Set direction
  long n_stepABS = abs(n_step);
  if (n_stepABS > 0x3FFFFF) n_stepABS = 0x3FFFFF;
  Xfer((byte)(n_stepABS >> 16));
  Xfer((byte)(n_stepABS >> 8));
  Xfer((byte)(n_stepABS));
}

// dSPIN_GOTO operates much like dSPIN_MOVE, except it produces absolute motion instead
//  of relative motion. The motor will be moved to the indicated position
//  in the shortest possible fashion.
void L6470::goTo(long pos) {
  Xfer(dSPIN_GOTO);
  if (pos > 0x3FFFFF) pos = 0x3FFFFF;
  Xfer((byte)(pos >> 16));
  Xfer((byte)(pos >> 8));
  Xfer((byte)(pos));
}

// Same as dSPIN_GOTO, but with user constrained rotational direction.
void L6470::goTo_DIR(const byte dir, long pos) {
  Xfer(dSPIN_GOTO_DIR | dir);
  if (pos > 0x3FFFFF) pos = 0x3FFFFF;
  Xfer((byte)(pos >> 16));
  Xfer((byte)(pos >> 8));
  Xfer((byte)(pos));
}

// GoUntil will set the motor running with direction dir (dSPIN_REV or
//  dSPIN_FWD) until a falling edge is detected on the SW pin. Depending
//  on bit SW_MODE in L6470_CONFIG, either a hard stop or a soft stop is
//  performed at the falling edge, and depending on the value of
//  act (either RESET or COPY) the value in the L6470_ABS_POS register is
//  either RESET to 0 or COPY-ed into the L6470_MARK register.
void L6470::goUntil(const byte act, const byte dir, unsigned long spd) {
  Xfer(dSPIN_GO_UNTIL | act | dir);
  if (spd > 0x3FFFFF) spd = 0x3FFFFF;
  Xfer((byte)(spd >> 16));
  Xfer((byte)(spd >> 8));
  Xfer((byte)(spd));
}

// Similar in nature to GoUntil, ReleaseSW produces motion at the
//  higher of two speeds: the value in L6470_MIN_SPEED or 5 steps/s.
//  The motor continues to run at this speed until a rising edge
//  is detected on the switch input, then a hard stop is performed
//  and the L6470_ABS_POS register is either COPY-ed into L6470_MARK or RESET to
//  0, depending on whether RESET or COPY was passed to the function
//  for act.
void L6470::releaseSW(const byte act, const byte dir) {
  Xfer(dSPIN_RELEASE_SW | act | dir);
}

// GoHome is equivalent to GoTo(0), but requires less time to send.
//  Note that no direction is provided; motion occurs through shortest
//  path. If a direction is required, use GoTo_DIR().
void L6470::goHome() { Xfer(dSPIN_GO_HOME); }

// GoMark is equivalent to GoTo(L6470_MARK), but requires less time to send.
//  Note that no direction is provided; motion occurs through shortest
//  path. If a direction is required, use GoTo_DIR().
void L6470::goMark() { Xfer(dSPIN_GO_MARK); }

void L6470::setMark(long value) {
  Xfer(L6470_MARK);
  if (value > 0x3FFFFF) value = 0x3FFFFF;
  if (value < -0x3FFFFF) value = -0x3FFFFF;
  Xfer((byte)(value >> 16));
  Xfer((byte)(value >> 8));
  Xfer((byte)(value));
}

void L6470::setMark() {
  long value = getPos();
  Xfer(L6470_MARK);
  if (value > 0x3FFFFF) value = 0x3FFFFF;
  if (value < -0x3FFFFF) value = -0x3FFFFF;
  Xfer((byte)(value >> 16));
  Xfer((byte)(value >> 8));
  Xfer((byte)(value));
}

// Sets the L6470_ABS_POS register to 0, effectively declaring the current
//  position to be "HOME".
void L6470::setAsHome() { Xfer(dSPIN_RESET_POS); }

// Reset device to power up conditions. Equivalent to toggling the STBY
//  pin or cycling power.
void L6470::resetDev() { Xfer(dSPIN_RESET_DEVICE); }

// Bring the motor to a halt using the deceleration curve.
void L6470::softStop() { Xfer(dSPIN_SOFT_STOP); }

// Stop the motor right away. No deceleration.
void L6470::hardStop() { Xfer(dSPIN_HARD_STOP); }

// Decelerate the motor and disengage
void L6470::softFree() { Xfer(dSPIN_SOFT_HIZ); }

// Disengage the motor immediately with no deceleration.
void L6470::free() { Xfer(dSPIN_HARD_HIZ); }

// Fetch and return the 16-bit value in the L6470_STATUS register. Resets
//  any warning flags and exits any error states. Using GetParam()
//  to read L6470_STATUS does not clear these values.
int L6470::getStatus() {
  Xfer(dSPIN_GET_STATUS);
  return Xfer(0) << 8 | Xfer(0);
}

// The value in the L6470_ACC register is [(steps/s/s)*(tick^2)]/(2^-40) where tick is
//  250ns (datasheet value)- 0x08A on boot.
// Multiply desired steps/s/s by .137438 to get an appropriate value for this register.
// This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
unsigned long L6470::AccCalc(const float stepsPerSecPerSec) {
  unsigned long temp = (unsigned long)(stepsPerSecPerSec * 0.137438);
  return temp < 0x00000FFF ? temp : 0x00000FFF;
}

unsigned long L6470::DecCalc(float stepsPerSecPerSec) {
  // The calculation for L6470_DEC is the same as for L6470_ACC. Value is 0x08A on boot.
  // This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
  unsigned long temp = (unsigned long)(stepsPerSecPerSec * 0.137438);
  return temp < 0x00000FFF ? temp : 0x00000FFF;
}

unsigned long L6470::MaxSpdCalc(const float stepsPerSec) {
  // The value in the MAX_SPD register is [(steps/s)*(tick)]/(2^-18) where tick is
  //  250ns (datasheet value)- 0x041 on boot.
  // Multiply desired steps/s by .065536 to get an appropriate value for this register
  // This is a 10-bit value, so we need to make sure it remains at or below 0x3FF
  unsigned long temp = (unsigned long)(stepsPerSec * .065536);
  return temp < 0x000003FF ? temp : 0x000003FF;
}

unsigned long L6470::MinSpdCalc(const float stepsPerSec) {
  // The value in the MIN_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is
  //  250ns (datasheet value)- 0x000 on boot.
  // Multiply desired steps/s by 4.1943 to get an appropriate value for this register
  // This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
  unsigned long temp = (unsigned long)(stepsPerSec * 4.1943);
  return temp < 0x00000FFF ? temp : 0x00000FFF;
}

unsigned long L6470::FSCalc(const float stepsPerSec) {
  // The value in the L6470_FS_SPD register is ([(steps/s)*(tick)]/(2^-18))-0.5 where tick is
  //  250ns (datasheet value)- 0x027 on boot.
  // Multiply desired steps/s by .065536 and subtract .5 to get an appropriate value for this register
  // This is a 10-bit value, so we need to make sure the value is at or below 0x3FF.
  unsigned long temp = (unsigned long)(stepsPerSec * .065536 - .5);
  return temp < 0x000003FF ? temp : 0x000003FF;
}

unsigned long L6470::IntSpdCalc(const float stepsPerSec) {
  // The value in the L6470_INT_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is
  //  250ns (datasheet value)- 0x408 on boot.
  // Multiply desired steps/s by 4.1943 to get an appropriate value for this register
  // This is a 14-bit value, so we need to make sure the value is at or below 0x3FFF.
  unsigned long temp = (unsigned long)(stepsPerSec * 4.1943);
  return temp < 0x00003FFF ? temp : 0x00003FFF;
}

unsigned long L6470::SpdCalc(const float stepsPerSec) {
  // When issuing dSPIN_RUN command, the 20-bit speed is [(steps/s)*(tick)]/(2^-28) where tick is
  //  250ns (datasheet value).
  // Multiply desired steps/s by 67.106 to get an appropriate value for this register
  // This is a 20-bit value, so we need to make sure the value is at or below 0xFFFFF.
  unsigned long temp = (unsigned long)(stepsPerSec * 67.106);
  return temp < 0x000FFFFF ? temp : 0x000FFFFF;
}

unsigned long L6470::Param(unsigned long value, const byte bit_len) {
  // Generalization of the subsections of the register read/write functionality.
  //  We want the end user to just write the value without worrying about length,
  //  so we pass a bit length parameter from the calling function.
  const byte byte_len = (bit_len + 7) / 8;
  // Ensure the value has no spurious bits set, and apply limit.
  unsigned long mask = 0xFFFFFFFF >> (32 - bit_len);
  if (value > mask) value = mask;
  // The following three if statements handle the various possible byte length
  //  transfers- it'll be no less than 1 but no more than 3 bytes of data.
  // L6470::Xfer() sends a byte out through SPI and returns a byte received
  //  over SPI- when calling it, we typecast a shifted version of the masked
  //  value, then we shift the received value back by the same amount and
  //  store it until return time.
  unsigned long ret_val;
  switch (byte_len) {
    case 3: ret_val = long(Xfer((byte)(value >> 16))) << 16; break;
    case 2: ret_val = long(Xfer((byte)(value >> 8))) << 8; break;
    case 1: ret_val = Xfer((byte)value); break;
    default: ret_val = 0; break;
  }
  //Serial.println(ret_val, HEX);

  // Return the received values. Mask off any unnecessary bits, just for
  //  the sake of thoroughness- we don't EXPECT to see anything outside
  //  the bit length range but better to be safe than sorry.
  return ret_val & mask;
}

byte L6470::Xfer(byte data) {
  // This simple function shifts a byte out over SPI and receives a byte over
  //  SPI. Unusually for SPI devices, the dSPIN requires a toggling of the
  //  CS (slaveSelect) pin after each byte sent. That makes this function
  //  a bit more reasonable, because we can include more functionality in it.
  digitalWrite(_SSPin, LOW);
  // SPI.transfer() both shifts a byte out on the MOSI pin AND receives a
  //  byte in on the MISO pin.
  const byte data_out = SPI.transfer(data);
  digitalWrite(_SSPin, HIGH);
  return data_out;
}

void L6470::SetParam(const byte param, const unsigned long value) {
  Xfer(dSPIN_SET_PARAM | param);
  ParamHandler(param, value);
}

// Read from the various registers in the dSPIN chip.
unsigned long L6470::GetParam(const byte param) {
  Xfer(dSPIN_GET_PARAM | param);
  return ParamHandler(param, 0);
}

long L6470::convert(unsigned long val) {
  // Convert 22bit 2s comp to signed long
  const int MSB = val >> 21;
  val >>= 11;
  val <<= 11;
  if (MSB == 1) val |= 0b11111111111000000000000000000000;
  return val;
}

// Much of the functionality between "get parameter" and "set parameter" is
//  very similar, so we deal with that by putting all of it in one function
//  here to save memory space and simplify the program.
unsigned long L6470::ParamHandler(const byte param, const unsigned long value) {
  // This switch structure handles the appropriate action for each register.
  //  This is necessary since not all registers are of the same length, either
  //  bit-wise or byte-wise, so we want to make sure we mask out any spurious
  //  bits and do the right number of transfers. That is handled by the dSPIN_Param()
  //  function, in most cases, but for 1-byte or smaller transfers, we call
  //  Xfer() directly.
  switch (param) {
    // L6470_ABS_POS is the current absolute offset from home. It is a 22 bit number expressed
    //  in two's complement. At power up, this value is 0. It cannot be written when
    //  the motor is running, but at any other time, it can be updated to change the
    //  interpreted position of the motor.
    case L6470_ABS_POS:    return Param(value, 22);
    // L6470_EL_POS is the current electrical position in the step generation cycle. It can
    //  be set when the motor is not in motion. Value is 0 on power up.
    case L6470_EL_POS:     return Param(value, 9);
    // L6470_MARK is a second position other than 0 that the motor can be told to go to. As
    //  with L6470_ABS_POS, it is 22-bit two's complement. Value is 0 on power up.
    case L6470_MARK:       return Param(value, 22);
    // L6470_SPEED contains information about the current speed. It is read-only. It does
    //  NOT provide direction information.
    case L6470_SPEED:      return Param(0, 20);
    // L6470_ACC and L6470_DEC set the acceleration and deceleration rates. Set L6470_ACC to 0xFFF
    //  to get infinite acceleration/decelaeration- there is no way to get infinite
    //  deceleration w/o infinite acceleration (except the HARD STOP command).
    //  Cannot be written while motor is running. Both default to 0x08A on power up.
    // AccCalc() and DecCalc() functions exist to convert steps/s/s values into
    //  12-bit values for these two registers.
    case L6470_ACC:
    case L6470_DEC:        return Param(value, 12);
    // L6470_MAX_SPEED is just what it says- any command which attempts to set the speed
    //  of the motor above this value will simply cause the motor to turn at this
    //  speed. Value is 0x041 on power up.
    // MaxSpdCalc() function exists to convert steps/s value into a 10-bit value
    //  for this register.
    case L6470_MAX_SPEED:  return Param(value, 10);
    // L6470_MIN_SPEED controls two things- the activation of the low-speed optimization
    //  feature and the lowest speed the motor will be allowed to operate at. LSPD_OPT
    //  is the 13th bit, and when it is set, the minimum allowed speed is automatically
    //  set to zero. This value is 0 on startup.
    // MinSpdCalc() function exists to convert steps/s value into a 12-bit value for this
    //  register. SetLowSpeedOpt() function exists to enable/disable the optimization feature.
    case L6470_MIN_SPEED:  return Param(value, 12);
    // L6470_FS_SPD register contains a threshold value above which microstepping is disabled
    //  and the dSPIN operates in full-step mode. Defaults to 0x027 on power up.
    // FSCalc() function exists to convert steps/s value into 10-bit integer for this
    //  register.
    case L6470_FS_SPD:     return Param(value, 10);
    // KVAL is the maximum voltage of the PWM outputs. These 8-bit values are ratiometric
    //  representations: 255 for full output voltage, 128 for half, etc. Default is 0x29.
    // The implications of different KVAL settings is too complex to dig into here, but
    //  it will usually work to max the value for dSPIN_RUN, L6470_ACC, and L6470_DEC. Maxing the value for
    //  HOLD may result in excessive power dissipation when the motor is not running.
    case L6470_KVAL_HOLD:  return Xfer((byte)value);
    case L6470_KVAL_RUN:   return Xfer((byte)value);
    case L6470_KVAL_ACC:   return Xfer((byte)value);
    case L6470_KVAL_DEC:   return Xfer((byte)value);
    // L6470_INT_SPD, L6470_ST_SLP, L6470_FN_SLP_ACC and L6470_FN_SLP_DEC are all related to the back EMF
    //  compensation functionality. Please see the datasheet for details of this
    //  function- it is too complex to discuss here. Default values seem to work
    //  well enough.
    case L6470_INT_SPD:    return Param(value, 14);
    case L6470_ST_SLP:     return Xfer((byte)value);
    case L6470_FN_SLP_ACC: return Xfer((byte)value);
    case L6470_FN_SLP_DEC: return Xfer((byte)value);
    // L6470_K_THERM is motor winding thermal drift compensation. Please see the datasheet
    //  for full details on operation- the default value should be okay for most users.
    case L6470_K_THERM:    return Xfer((byte)value & 0x0F);
    // L6470_ADC_OUT is a read-only register containing the result of the ADC measurements.
    //  This is less useful than it sounds; see the datasheet for more information.
    case L6470_ADC_OUT:    return Xfer(0);
    // Set the overcurrent threshold. Ranges from 375mA to 6A in steps of 375mA.
    //  A set of defined constants is provided for the user's convenience. Default
    //  value is 3.375A- 0x08. This is a 4-bit value.
    case L6470_OCD_TH:     return Xfer((byte)value & 0x0F);
    // Stall current threshold. Defaults to 0x40, or 2.03A. Value is from 31.25mA to
    //  4A in 31.25mA steps. This is a 7-bit value.
    case L6470_STALL_TH:   return Xfer((byte)value & 0x7F);
    // L6470_STEP_MODE controls the microstepping settings, as well as the generation of an
    //  output signal from the dSPIN. Bits 2:0 control the number of microsteps per
    //  step the part will generate. Bit 7 controls whether the BUSY/SYNC pin outputs
    //  a BUSY signal or a step synchronization signal. Bits 6:4 control the frequency
    //  of the output signal relative to the full-step frequency; see datasheet for
    //  that relationship as it is too complex to reproduce here.
    // Most likely, only the microsteps per step value will be needed; there is a set
    //  of constants provided for ease of use of these values.
    case L6470_STEP_MODE:  return Xfer((byte)value);
    // L6470_ALARM_EN controls which alarms will cause the FLAG pin to fall. A set of constants
    //  is provided to make this easy to interpret. By default, ALL alarms will trigger the
    //  FLAG pin.
    case L6470_ALARM_EN:   return Xfer((byte)value);
    // L6470_CONFIG contains some assorted configuration bits and fields. A fairly comprehensive
    //  set of reasonably self-explanatory constants is provided, but users should refer
    //  to the datasheet before modifying the contents of this register to be certain they
    //  understand the implications of their modifications. Value on boot is 0x2E88; this
    //  can be a useful way to verify proper start up and operation of the dSPIN chip.
    case L6470_CONFIG:     return Param(value, 16);
    // L6470_STATUS contains read-only information about the current condition of the chip. A
    //  comprehensive set of constants for masking and testing this register is provided, but
    //  users should refer to the datasheet to ensure that they fully understand each one of
    //  the bits in the register.
    case L6470_STATUS:     return Param(0, 16);  // L6470_STATUS is a read-only register
  }
  return Xfer((byte)(value));
}
