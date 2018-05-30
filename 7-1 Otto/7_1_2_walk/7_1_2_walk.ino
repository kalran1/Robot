#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h> 

// 아래의 6개설정은 사용자 환경에 맞게 수정하세요.
const char* ssid = "AndroidHotspot7147"; // 와이파이 AP, 또는 스마트폰의 핫스판 이름
const char* password = "01033887147";  // 와이파이 AP, 또는 스마트폰의 핫스판 이름
const char* mqtt_server = "broker.mqtt-dashboard.com"; //브로커 주소
const char* outTopic = "/kdi/inLight"; // 이름이 중복되지 않게 설정 기록
const char* inTopic = "/kdi/outLight"; // 이름이 중복되지 않게 설정 기록
const char* clientName = "943Client";  // 다음 이름이 중복되지 않게 꼭 수정 바람 - 생년월일 추천

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
Servo servoLeft1;
Servo servoLeft2;
Servo servoRight1;
Servo servoRight2;
int data[100],qData;  //qData = data[]의 개수
int iTime=500;  // 로봇 동작속도

int Duty;
char imode='0'; //imode 0=초기치 1,2,3,4=서보모터 회전 5=동작속도 9=DoArray
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  servoLeft1.attach(5);  // attaches the servo on GIO5 to the servo object 
  servoLeft2.attach(4);  // 
  servoRight1.attach(0); 
  servoRight2.attach(2); 

}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// 통신에서 문자가 들어오면 이 함수의 payload 배열에 저장된다.
void callback(char* topic, byte* payload, unsigned int length) {
  int i,j,k;
  char preMode;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  preMode = imode;
  imode = payload[0];
  char szHex[3];
  for (i = 1; i < 4; i++) 
    szHex[i-1] = (char)payload[i];
  Duty = (int)strtol(szHex, NULL, 10);

  if((char)payload[0]=='0')
    InitSet();
  else if((char)payload[0]=='1')
    servoLeft1.write(Duty);
  else if((char)payload[0]=='2')
    servoLeft2.write(Duty);
  else if((char)payload[0]=='3')
    servoRight1.write(Duty);
  else if((char)payload[0]=='4')
    servoRight2.write(Duty);
  else if((char)payload[0]=='5') {
    Serial.print(imode);
    iTime = Duty;
    imode = preMode;
  }
  else
    Serial.print("No choice");
  Serial.print(imode);
  
  //download
   if((char)payload[0]=='9') { 
    payload[0]=0x00;
    j=0;
    for(i=0;i<length;i++) {
      if(payload[i]>47 && payload[i]<58) {
        payload[j] = payload[i];
        j++;
      }
    }
    k=j/3;
    qData=k;
    for (j=0; j<k ;j++) {
      szHex[0] = (char)payload[j*3];
      szHex[1] = (char)payload[j*3+1];
      szHex[2] = (char)payload[j*3+2];
      data[j] = (int)strtol(szHex, NULL, 10);
    }
    Serial.println(qData);
    Serial.println("=======");
    for(i=0;i<k;i++)
      Serial.println(data[i]);
  }
    
}

// mqtt 통신에 지속적으로 접속한다.
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "Reconnected");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void DoArray() {
  int i,j,k;
  k=qData/4;
  for(i=0 ; i<qData ; i++) {
    j=i%4;
    Serial.println(j);
    Serial.println(data[i]);
    if(j==0)
       servoLeft1.write(data[i]);
    else if(j==1)
       servoLeft2.write(data[i]);
    else if(j==2)
       servoRight1.write(data[i]);
    else if(j==3)
       servoRight2.write(data[i]);

      delay(iTime);
  }
}

void InitSet() {
  servoLeft1.write(90);
  delay(iTime);
  servoLeft2.write(90);
  delay(iTime);
  servoRight1.write(90);
  delay(iTime);  
  servoRight2.write(90);
  delay(iTime);   
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(imode=='9')
    DoArray();
}
