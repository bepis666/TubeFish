//#pragma once

#ifndef FishTubeMotor_h
#define FishTubeMotor_h

#include <Arduino.h>
#include <Servo.h>


class FishTubeMotor {

  public:
  
    bool motor_has_been_initialized = false;
    int output = 0;
    int pwm = 0;

    void InitMotor(int sp);
    void Move(int in_setpoint);

    FishTubeMotor(int init_servo_pin, int init_sampling_time, int init_acceleration_time, int init_servo_minimum, int init_servo_middle, int init_servo_maximum);
	

  private:
  
	Servo motor;

    int servo_pin;

    uint32_t sampling_time;
    int servo_minimum;
	int servo_middle;
    int servo_maximum;

    int setpoint_minimum;
	int setpoint_middle;
    int setpoint_maximum;
    int output_minimum;
    int output_maximum;

	int acceleration;
	
    int setpoint = 0;
    uint32_t last_sampling_time = 0;
	

};

#endif