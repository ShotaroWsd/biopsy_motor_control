#include <IRROBOT_ServoTesterShield.h>

IRROBOT_ServoTesterShield Tester(&Serial1);
#define APPLICATION_VR      Tester.VR_1 
#define MANUAL_POSITION_VR  Tester.VR_2 
#define MIN_STROKE_VR       Tester.VR_3 
#define MAX_STROKE_VR       Tester.VR_4 
#define SPEED_VR            Tester.VR_5 
#define DELAY_VR            Tester.VR_6 

#define MAX_STROKE 2075
#define MIN_STROKE 1250
#define RECIEVE_DATA_SIZE 5
#define SEND_DATA_SIZE 3

uint8_t recieve_data[RECIEVE_DATA_SIZE];
uint8_t send_data[SEND_DATA_SIZE];
int position_val;  // モータの位置目標
int position_final;  //  位置制御の最終目標位置
int sleep_time_ms = 0;  
int speed_val;  // 受信したスピード


void setup() {
  Serial1.begin(9600);
  Tester.begin();
  Tester.servo_CH1.writeMicroseconds(1500);
  while(!Serial);
  Tester.servo_CH1.writeMicroseconds(MAX_STROKE);
  position_val = MAX_STROKE;
}


void loop() {
  if(Serial.available()){
    Serial.readBytes(recieve_data, RECIEVE_DATA_SIZE);
    send_data[0] = 0;
    send_data[1] = position_val;
    send_data[2] = position_val >> 8;

    switch(recieve_data[0]){
      case 0:  // 力制御
        position_val = recieve_data[2] << 8 | recieve_data[1];
        break;
      case 1:  // 位置制御
        position_final = recieve_data[2] << 8 | recieve_data[1];
        speed_val = recieve_data[4] << 8 | recieve_data[3];
        sleep_time_ms = (1023 - speed_val) / 80;  // loopごとのスリーブ時間
        break;
      case 2:  // 停止
        Tester.servo_CH1.writeMicroseconds(2060);
        delay(50);
        Tester.servo_CH1.detach();
        exit(1);
        break;
    }
    
    Serial.write(send_data, SEND_DATA_SIZE);
    Serial.flush();
    
  }

  // ストローク制限
  if(position_val > MAX_STROKE){
    position_val = MAX_STROKE;  
  }else if(position_val < MIN_STROKE){
    position_val = MIN_STROKE;  
  }

  if(recieve_data[0] == 1 && position_val >= position_final){
    position_val--;  // 位置制御時の目標位置変更
  }
  
  Tester.servo_CH1.write(position_val);
  delay(sleep_time_ms);
  
  if(Tester.MODE_0.read()){  // スイッチで停止
    exit(1);
    }
}
