#include "FishTubeMotor.h"

// Конструктор
FishTubeMotor::FishTubeMotor(int init_servo_pin, int init_sampling_time, int init_acceleration_time, int init_servo_minimum, int init_servo_middle, int init_servo_maximum) {

  servo_pin = init_servo_pin;

  servo_minimum = init_servo_minimum;
  servo_middle = init_servo_middle;
  servo_maximum = init_servo_maximum;

  setpoint_minimum = servo_minimum;
  setpoint_middle = servo_middle;
  setpoint_maximum = servo_maximum;

  sampling_time = init_sampling_time;
  acceleration = (servo_maximum - servo_middle) / (init_acceleration_time / sampling_time);
  last_sampling_time = millis();
  
  output = servo_middle;
  pwm = servo_middle;
}

// Инициализация двигателя
void FishTubeMotor::InitMotor(int sp) {
	
	motor.attach(servo_pin, servo_minimum, servo_maximum);
	motor.writeMicroseconds(servo_middle);

}
 
// Задание двигателя
void FishTubeMotor::Move(int in_setpoint) {
	
	
  // Если двигатель инициализирован
  if (motor_has_been_initialized) {

    //Нормализация значения уставки
    setpoint = constrain(in_setpoint, -100, 100);
    setpoint = map(setpoint, -100, 100, setpoint_minimum, setpoint_maximum);

    //Изменение задания двигателя происходит через время заданное переменной sampling_time
    if ((millis() - last_sampling_time) >= sampling_time) {
      last_sampling_time = millis();

      //Ускорение
      if (setpoint != output) {

        if (setpoint < output) {
		  output = output - acceleration;
          output_minimum = setpoint;
          output_maximum = output;
		  
        }

        else {
		  output = output + acceleration;
          output_minimum = output;
          output_maximum = setpoint;
        }
		
        output = constrain(output, output_minimum, output_maximum);
      }

      //Подача PWM
      pwm = map(output, setpoint_minimum, setpoint_maximum, servo_minimum, servo_maximum);
      motor.writeMicroseconds(pwm);
    }
  }
}
