#include <Servo.h>
#include <Wire.h>

#define MAX_SENSER_VAL 755
#define MIN_SENSER_VAL 268
#define MAX_SERVO_VAL 180
#define MIN_SERVO_VAL 0
#define I2C_CHANNEL 8

// A4(SDA),A5(SCL)は、I2C通信に使用する
// リモコンスティック操作モード2 mode2を例としてPINを以下の通りにする
Servo servo_throttle;  // D2PIN 左スティック上下 スロットル throttle
Servo servo_ladder;  // D3PIN 左スティック左右 ラダー ladder
Servo servo_elevator;  // D4PIN 右スティック上下 エレベーター elevator
Servo servo_aileron;  // D5PIN 右スティック左右 エルロン aileron
static char i2c_throttle = 'Z';  // i2cから受け取るスティックの値
static char i2c_ladder = 'Z';
static char i2c_elevator = 'Z';
static char i2c_aileron = 'Z';

void setup(){
  // D0(RX),D1(TX)は、UART通信に使用する
  servo_throttle.attach(2, 1500 - 500, 1500 + 500);  // D2ピンを信号線として設定
  servo_ladder.attach(3, 1500 - 500, 1500 + 500);
  servo_elevator.attach(4, 1500 - 500, 1500 + 500);
  servo_aileron.attach(5, 1500 - 500, 1500 + 500);
  Wire.begin(I2C_CHANNEL);  // join i2c bus with address
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(115200);
  Serial.println("Start!");
}

void loop(){
  if (i2c_throttle == 'Z' && i2c_ladder == 'Z' && i2c_elevator == 'Z' && i2c_aileron == 'Z'){
    int angle_throttle = map(analogRead(A0), MIN_SENSER_VAL, MAX_SENSER_VAL, MIN_SERVO_VAL, MAX_SERVO_VAL);  // センター515,最小269,最大754
    int angle_ladder = map(analogRead(A1), MIN_SENSER_VAL, MAX_SENSER_VAL, MIN_SERVO_VAL, MAX_SERVO_VAL);
    int angle_elevator = map(analogRead(A2), MIN_SENSER_VAL, MAX_SENSER_VAL, MIN_SERVO_VAL, MAX_SERVO_VAL);  // センター508,最小272,最大750
    int angle_aileron = map(analogRead(A3), MIN_SENSER_VAL, MAX_SENSER_VAL, MIN_SERVO_VAL, MAX_SERVO_VAL);
    angle_throttle = constrain(angle_throttle, MIN_SERVO_VAL, MAX_SERVO_VAL);
    angle_ladder = constrain(angle_ladder, MIN_SERVO_VAL, MAX_SERVO_VAL);
    angle_elevator = constrain(angle_elevator, MIN_SERVO_VAL, MAX_SERVO_VAL);
    angle_aileron = constrain(angle_aileron, MIN_SERVO_VAL, MAX_SERVO_VAL);
    servo_throttle.write(angle_throttle);
    servo_ladder.write(angle_ladder);
    servo_elevator.write(angle_elevator);
    servo_aileron.write(angle_aileron);
    Serial.print((char)map(angle_throttle, MIN_SERVO_VAL, MAX_SERVO_VAL, 'A', 'O'));
    Serial.print((char)map(angle_ladder, MIN_SERVO_VAL, MAX_SERVO_VAL, 'A', 'O'));
    Serial.print((char)map(angle_elevator, MIN_SERVO_VAL, MAX_SERVO_VAL, 'A', 'O'));
    Serial.print((char)map(angle_aileron, MIN_SERVO_VAL, MAX_SERVO_VAL, 'A', 'O'));
    Serial.print(F("\r\n"));
  }
}

void receiveEvent(int howMany){
  String str = "";
  while (Wire.available()) {
    char c = Wire.read();
    str += String(c);
    Serial.print(c);
  }
  Serial.print(F("\r\n"));
  i2c_ladder = str.charAt(0);  // l_x
  i2c_throttle = str.charAt(1);  // l_y
  i2c_aileron = str.charAt(2);  // r_x
  i2c_elevator = str.charAt(3);  // r_y
  servo_throttle.write(map(i2c_throttle, 'A', 'O', MIN_SERVO_VAL, MAX_SERVO_VAL));
  servo_ladder.write(map(i2c_ladder, 'A', 'O', MIN_SERVO_VAL, MAX_SERVO_VAL));
  servo_elevator.write(map(i2c_elevator, 'A', 'O', MIN_SERVO_VAL, MAX_SERVO_VAL));
  servo_aileron.write(map(i2c_aileron, 'A', 'O', MIN_SERVO_VAL, MAX_SERVO_VAL));
}
