#include <Wire.h>
#include <FishTubeMotor.h>

// Motor's settings
#define Motor_1_pin 3
#define Motor_2_pin 9
#define Motor_3_pin 10
#define Motor_4_pin 11
#define Motor_sampling_time 50
#define Motor_acceleration_time 2000
#define Motor_pwm_min 965
#define Motor_pwm_middle 1465
#define Motor_pwm_max 1965

//I2C settings
#define I2C_address 11
#define I2C_timeout 2000

// ----- Motor's objects -----

// Vertical Left
FishTubeMotor FishTubeMotor_1(Motor_1_pin, Motor_sampling_time, Motor_acceleration_time, Motor_pwm_min, Motor_pwm_middle, Motor_pwm_max);
// Vertical Right
FishTubeMotor FishTubeMotor_2(Motor_2_pin, Motor_sampling_time, Motor_acceleration_time, Motor_pwm_min, Motor_pwm_middle, Motor_pwm_max);
// Horizontal Left
FishTubeMotor FishTubeMotor_3(Motor_3_pin, Motor_sampling_time, Motor_acceleration_time, Motor_pwm_min, Motor_pwm_middle, Motor_pwm_max);
// Horizontal Right
FishTubeMotor FishTubeMotor_4(Motor_4_pin, Motor_sampling_time, Motor_acceleration_time, Motor_pwm_min, Motor_pwm_middle, Motor_pwm_max);

int8_t Motor_1_setpoint = 0;
int8_t Motor_2_setpoint = 0;
int8_t Motor_3_setpoint = 0;
int8_t Motor_4_setpoint = 0;

// ----- I2C -----
/* 
  0 - status code
  1 - vertical left setpoint   (-100...100 => 0...200)
  2 - vertical right setpoint  (-100...100 => 0...200)
  3 - horizontal left setpoint (-100...100 => 0...200)
  4 - horizontal left setpoint (-100...100 => 0...200)
  5 - engines full speed (0 - max speed 10%, 1 - max speed 100%)
 */
byte I2C_registers[10] = {0, 100, 100, 100, 100, 0};

bool communication_status = 0;
uint32_t last_communication_time = 0;

byte I2C_message_lenght = 0;
byte I2C_cmd = 0;

uint32_t time_ = 0;

void setup() {

  // COM port initialization
  // Serial.begin(9600);

  // I2C initialization. Device address is 11
  Wire.begin(I2C_address);

  // I2C receive and request functions attaching
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // Serial.println("setpoint, pwm");

  //Motors initialization
  delay(3000);
  FishTubeMotor_1.InitMotor(1465);
  FishTubeMotor_2.InitMotor(1465);
  FishTubeMotor_3.InitMotor(1465);
  FishTubeMotor_4.InitMotor(1465);
  delay(3000);
  FishTubeMotor_1.motor_has_been_initialized = true;
  FishTubeMotor_2.motor_has_been_initialized = true;
  FishTubeMotor_3.motor_has_been_initialized = true;
  FishTubeMotor_4.motor_has_been_initialized = true;

  // for (byte i = 0; i < 10; i++) {
    // Serial.print(I2C_registers[i]);
    // Serial.print(",");
  // }

}

void loop() {

  // Motors moving
  if (communication_status) {

    // I2C communication status
    if ((millis() - last_communication_time) > 2000) {
      communication_status = 0;
      Motor_1_setpoint = 0;
      Motor_2_setpoint = 0;
      Motor_3_setpoint = 0;
      Motor_4_setpoint = 0;
      // Serial.println("connection lost");
    }

    FishTubeMotor_1.Move(Motor_1_setpoint);
    FishTubeMotor_2.Move(Motor_2_setpoint);
    FishTubeMotor_3.Move(Motor_3_setpoint);
    FishTubeMotor_4.Move(Motor_4_setpoint);

  }
  else {
    FishTubeMotor_1.Move(0);
    FishTubeMotor_2.Move(0);
    FishTubeMotor_3.Move(0);
    FishTubeMotor_4.Move(0);
  }

}


void receiveEvent() {

  communication_status = 1;
  last_communication_time = millis();

  I2C_message_lenght = Wire.available() - 1;
  I2C_cmd = Wire.read();

  //  Serial.print(I2C_cmd);
  //  Serial.println();

  if (I2C_message_lenght > 0) {
    for (byte i = 0; i < I2C_message_lenght; i++) {
      I2C_registers[(I2C_cmd + i)] = Wire.read();
      //      Serial.print(I2C_registers[(I2C_cmd + i)]);
      //      Serial.println();
    }
  }

  setpoint_convertation();

  //  for (byte i = 0; i < 10; i++) {
  //    Serial.print(I2C_registers[i]);
  //    Serial.print(",");
  //  }
  //
  //  Serial.println();
  //  Serial.println("------I2C MESSAGE RECEIVE END-------");
}

void requestEvent() {

  communication_status = 1;
  last_communication_time = millis();

  if (I2C_cmd == 0) {
    for (byte i = 0; i < 10; i++) {
      Wire.write(I2C_registers[(I2C_cmd + i)]);
    }
  }

  //  Serial.println();
  //  Serial.println("------I2C MESSAGE REQUEST END-------");
}

void setpoint_convertation() {

  Motor_1_setpoint = I2C_registers[1] - 100;
  Motor_2_setpoint = I2C_registers[2] - 100;
  Motor_3_setpoint = I2C_registers[3] - 100;
  Motor_4_setpoint = I2C_registers[4] - 100;

  if (I2C_registers[5] == 0) {

    Motor_1_setpoint = Motor_1_setpoint / 10;
    Motor_2_setpoint = Motor_2_setpoint / 10;
    Motor_3_setpoint = Motor_3_setpoint / 10;
    Motor_4_setpoint = Motor_4_setpoint / 10;

  }

}
