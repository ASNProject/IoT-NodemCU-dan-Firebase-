#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

int data1 = 0;
int data2 = 0;
int data3 = 0;
int saverelay1 = 0;

const char* ssid = "wifi";
const char* password = "1234567890";

const long utcOffsetInSeconds = 25200;
//Week Days
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", utcOffsetInSeconds);
String formattedDate;
String dayStamp;
String timeStamp;

int sepersepuluhDetik = 0;
int detik = 0;
int menit = 0;
int jam = 0;

int state = 0;

unsigned long interval = 1000;
unsigned long previousMillis = 0;

FirebaseData firebaseData;
FirebaseData ledData;

String dt[10];
int i;
String waktu;
boolean parsing = false;

unsigned long mulai, selesai, dataStopWatch;
int x = 0;
int fPaus = 0;
long lastButton = 0;
long delayAntiBouncing = 50;
long dataPaus = 0;

#include <dht.h>
#define sensor 14
#define LDR1 12
#define LDR2 13
#define relay1 5
#define relay2 16
#define relay3 4

dht DHT;
#define relay D1
int lampuNyala = LOW;
int lampuMati = HIGH;
char perintah;

int nilaisensor1;
int nilaisensor2;


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  konekWifi();
  Firebase.begin("https://iotse-b1c10-default-rtdb.firebaseio.com/", "6GLI4O3qpvLFMrMM1XamHDi8kiUrWiYmU7AsDNA1");
  timeClient.begin();

  //PINMODE
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
}


void konekWifi() {
  WiFi.begin(ssid, password);
  float c = 0;
  float n = 0;
  //memulai menghubungkan ke wifi router
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); //status saat mengkoneksikan
    Firebase.setString(firebaseData, "Status", "Not Connected");
  }

  Firebase.setString(firebaseData, "Status", "Connected");
  Serial.println("Sukses terkoneksi wifi!");
  Serial.println("IP Address:"); //alamat ip lokal
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentMillis = millis();
  pump();
  timeClient.update();
  simpandata();

  if ((unsigned long)(currentMillis - previousMillis) >= interval) {
    DHT.read11(sensor);
    Firebase.setFloat(firebaseData, "humidity", DHT.humidity);
    Firebase.setFloat(firebaseData, "temperature", DHT.temperature);
    previousMillis = millis();

  }

   /////////////////////////---POTONGAN PROGRAM HALAMAN---//////////////////////////////////
  if (Firebase.getString(firebaseData, "/modehalaman")) {
    if (firebaseData.dataType() == "string")
    {
      String HalamanMode = firebaseData.stringData();
      if (HalamanMode == "AUTO") {
        //Mode Automatis menggunakan sensor atau manual
        halaman();
      }
      if (HalamanMode == "TIMER") {
        int j = timeClient.getHours();
        int m = timeClient.getMinutes();
        //Mode Timer menggunakan waktu yang sudah disetting pada Website
        if (Firebase.getString(firebaseData, "/timeronhalaman")) {
          if (firebaseData.dataType() == "string")
          {
            String waktuon = "*" + firebaseData.stringData() + "#";
            int j = 0;
            dt[j] = "";

            for (i = 1; i < waktuon.length(); i++) {
              if ((waktuon[i] == '#') || (waktuon[i] == '.'))
              {
                j++;
                dt[j] = "";
              }
              else
              {
                dt[j] = dt[j] + waktuon[i];
              }
            }
            int jamon = dt[0].toInt();
            int meniton = dt[1].toInt();
            int dektikon = dt[2].toInt();
            if (j == jamon && m == meniton) {
              digitalWrite(relay1, LOW);
              Firebase.setString(firebaseData, "statusrelay1", "HIDUP");
            }
          }
        }
        if (Firebase.getString(firebaseData, "/timeroffhalaman")) {
          if (firebaseData.dataType() == "string")
          {
            String waktuoff = "*" + firebaseData.stringData() + "#";
            int j = 0;
            dt[j] = "";

            for (i = 1; i < waktuoff.length(); i++) {
              if ((waktuoff[i] == '#') || (waktuoff[i] == '.'))
              {
                j++;
                dt[j] = "";
              }
              else
              {
                dt[j] = dt[j] + waktuoff[i];
              }
            }
            int jamoff = dt[0].toInt();
            int menitoff = dt[1].toInt();
            int dektikoff = dt[2].toInt();



            if (j == jamoff && m == menitoff) {
              digitalWrite(relay1, HIGH);
              Firebase.setString(firebaseData, "statusrelay1", "MATI");
            }
          }

        }

      }
    }
  }
  if (Firebase.getString(firebaseData, "/statusrelay1")) {
    if (firebaseData.dataType() == "string")
    {
      String relaystatus = firebaseData.stringData();
      if (relaystatus == "HIDUP") {
        Firebase.setString(firebaseData, "statuscolor", "green");
      }
      else if (relaystatus == "MATI") {
        Firebase.setString(firebaseData, "statuscolor", "red");
      }
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////---POTONGAN PROGRAM RUANGTAMU---//////////////////////////////////
  if (Firebase.getString(firebaseData, "/modelampu")) {
    if (firebaseData.dataType() == "string")
    {
      String RuangtamuMode = firebaseData.stringData();
      if (RuangtamuMode == "AUTO") {
        //Mode Automatis menggunakan sensor atau manual
        ruangtamu();
      }
      else if (RuangtamuMode == "TIMER") {
        //Mode Timer menggunakan waktu yang sudah disetting pada Website
        int j = timeClient.getHours();
        int m = timeClient.getMinutes();
        //Mode Timer menggunakan waktu yang sudah disetting pada Website
        if (Firebase.getString(firebaseData, "/timeronruangtamu")) {
          if (firebaseData.dataType() == "string")
          {
            String waktuon = "*" + firebaseData.stringData() + "#";
            int j = 0;
            dt[j] = "";

            for (i = 1; i < waktuon.length(); i++) {
              if ((waktuon[i] == '#') || (waktuon[i] == '.'))
              {
                j++;
                dt[j] = "";
              }
              else
              {
                dt[j] = dt[j] + waktuon[i];
              }
            }
            int jamon = dt[0].toInt();
            int meniton = dt[1].toInt();
            int dektikon = dt[2].toInt();
            if (j == jamon && m == meniton) {
              digitalWrite(relay2, LOW);
              Firebase.setString(firebaseData, "statusrelay2", "HIDUP");
            }
          }
        }
        if (Firebase.getString(firebaseData, "/timeroffruangtamu")) {
          if (firebaseData.dataType() == "string")
          {
            String waktuoff = "*" + firebaseData.stringData() + "#";
            int j = 0;
            dt[j] = "";

            for (i = 1; i < waktuoff.length(); i++) {
              if ((waktuoff[i] == '#') || (waktuoff[i] == '.'))
              {
                j++;
                dt[j] = "";
              }
              else
              {
                dt[j] = dt[j] + waktuoff[i];
              }
            }
            int jamoff = dt[0].toInt();
            int menitoff = dt[1].toInt();
            int dektikoff = dt[2].toInt();

            if (j == jamoff && m == menitoff) {
              digitalWrite(relay2, HIGH);
              Firebase.setString(firebaseData, "statusrelay2", "MATI");
            }
          }

        }
      }
    }
  }

  if (Firebase.getString(firebaseData, "/statusrelay2")) {
    if (firebaseData.dataType() == "string")
    {
      String relaystatus2 = firebaseData.stringData();
      if (relaystatus2 == "HIDUP") {
        Firebase.setString(firebaseData, "statuscolor2", "green");
      }
      else if (relaystatus2 == "MATI") {
        Firebase.setString(firebaseData, "statuscolor2", "red");
      }
    }
  }

  /////////////////////////---POTONGAN PROGRAM PUMP---//////////////////////////////////
  if (Firebase.getString(firebaseData, "/modepump")) {
    if (firebaseData.dataType() == "string")
    {
      String HalamanMode = firebaseData.stringData();
      if (HalamanMode == "AUTO") {
        //Mode Automatis menggunakan sensor atau manual
        pump();
      }
      if (HalamanMode == "TIMER") {
        int j = timeClient.getHours();
        int m = timeClient.getMinutes();
        //Mode Timer menggunakan waktu yang sudah disetting pada Website
        if (Firebase.getString(firebaseData, "/timeronpump")) {
          if (firebaseData.dataType() == "string")
          {
            String waktuon = "*" + firebaseData.stringData() + "#";
            int j = 0;
            dt[j] = "";

            for (i = 1; i < waktuon.length(); i++) {
              if ((waktuon[i] == '#') || (waktuon[i] == '.'))
              {
                j++;
                dt[j] = "";
              }
              else
              {
                dt[j] = dt[j] + waktuon[i];
              }
            }
            int jamon = dt[0].toInt();
            int meniton = dt[1].toInt();
            int dektikon = dt[2].toInt();
            if (j == jamon && m == meniton) {
              digitalWrite(relay3, LOW);
              Firebase.setString(firebaseData, "statusrelay3", "HIDUP");
            }
          }
        }
        if (Firebase.getString(firebaseData, "/timeroffpump")) {
          if (firebaseData.dataType() == "string")
          {
            String waktuoff = "*" + firebaseData.stringData() + "#";
            int j = 0;
            dt[j] = "";

            for (i = 1; i < waktuoff.length(); i++) {
              if ((waktuoff[i] == '#') || (waktuoff[i] == '.'))
              {
                j++;
                dt[j] = "";
              }
              else
              {
                dt[j] = dt[j] + waktuoff[i];
              }
            }
            int jamoff = dt[0].toInt();
            int menitoff = dt[1].toInt();
            int dektikoff = dt[2].toInt();



            if (j == jamoff && m == menitoff) {
              digitalWrite(relay3, HIGH);
              Firebase.setString(firebaseData, "statusrelay3", "MATI");
            }
          }

        }

      }
    }
  }
  if (Firebase.getString(firebaseData, "/statusrelay3")) {
    if (firebaseData.dataType() == "string")
    {
      String relaystatus = firebaseData.stringData();
      if (relaystatus == "HIDUP") {
        Firebase.setString(firebaseData, "statuscolor3", "green");
      }
      else if (relaystatus == "MATI") {
        Firebase.setString(firebaseData, "statuscolor3", "red");
      }
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////////////////////
void iotcontrol() {

  nilaisensor1 = digitalRead(LDR1);
  Firebase.setInt(firebaseData, "sensor1", nilaisensor1);
  if (nilaisensor1 == 0) {
    digitalWrite(relay1, HIGH);
    Firebase.setString(firebaseData, "statusrelay1", "MATI");
  }
  if (nilaisensor1 == 1) {
    digitalWrite(relay1, LOW);
    Firebase.setString(firebaseData, "statusrelay1", "HIDUP");
  }
  delay(1000);
}


///////////////////////////////////////////////////////////////////////////////////////////
void iotcontrol2() {

  nilaisensor2 = digitalRead(LDR2);
  Firebase.setInt(firebaseData, "sensor2", nilaisensor2);
  if (nilaisensor2 == 0) {
    digitalWrite(relay2, HIGH);
    Firebase.setString(firebaseData, "statusrelay2", "MATI");
  }
  if (nilaisensor2 == 1) {
    digitalWrite(relay2, LOW);
    Firebase.setString(firebaseData, "statusrelay2", "HIDUP");
  }
  delay(1000);
}


//////////////////////////////////////////////////////////////////////////////////////////
void pump() {
  //ROOM1
  if (Firebase.getString(firebaseData, "/relay3")) { //misal database diberikan nama relay1
    if  (firebaseData.dataType() == "string")
    {
      String Relay3 = firebaseData.stringData();
      if (Relay3 == "ON") {
        digitalWrite(relay3, LOW);
        Firebase.setString(firebaseData, "statusrelay3", "HIDUP");
      }
      else if (Relay3 == "OFF") {
        digitalWrite(relay3, HIGH);
        Firebase.setString(firebaseData, "statusrelay3", "MATI");


      }
      else {
        Serial.println("Salah kode! isi dengan data ON/OFF");
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////////////////
void halaman() {
  if (Firebase.getString(firebaseData, "/relay1")) { //misal database diberikan nama relay1
    if  (firebaseData.dataType() == "string")
    {
      String Relay1 = firebaseData.stringData();

      if (Relay1 == "ON") {
        digitalWrite(relay1, LOW);
        Firebase.setString(firebaseData, "statusrelay1", "HIDUP");

      }
      else if (Relay1 == "OFF") {
        digitalWrite(relay1, HIGH);
        Firebase.setString(firebaseData, "statusrelay1", "MATI");
        iotcontrol();

      }
      else {
        Serial.println("Salah kode! isi dengan data ON/OFF");
      }
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////
void ruangtamu() {
  if (Firebase.getString(firebaseData, "/relay2")) { //misal database diberikan nama relay1
    if  (firebaseData.dataType() == "string")
    {
      String Relay2 = firebaseData.stringData();
      if (Relay2 == "ON") {
        digitalWrite(relay2, LOW);
        Firebase.setString(firebaseData, "statusrelay2", "HIDUP");
      }
      else if (Relay2 == "OFF") {
        digitalWrite(relay2, HIGH);

        Firebase.setString(firebaseData, "statusrelay2", "MATI");
        iotcontrol2();
      }
      else {
        Serial.println("Salah kode! isi dengan data ON/OFF");
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////////////////////////
void simpandata() {
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();

  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  //Nama Hari
  String weekDay = weekDays[timeClient.getDay()];
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  //Hari ke dalam bulan
  int monthDay = ptm->tm_mday;
  //Bulan ke
  int currentMonth = ptm->tm_mon + 1;
  //Nama Bulan saat ini
  String currentMonthName = months[currentMonth - 1];
  //Tahun Saat ini
  int currentYear = ptm->tm_year + 1900;
  String years = String(currentYear);
  ////////////////SAVE DATA//////////////////////////////
  if (currentHour == 00 && currentMinute == 00) {
    int i = 0;
    EEPROM.put(data1, i);
    data1 += sizeof(int);
    EEPROM.put(data1, i);
    EEPROM.commit();
    Firebase.setString(firebaseData, "timerelay1", "0" );
    Firebase.setString(firebaseData, "timerelay2", "0" );
    Firebase.setString(firebaseData, "timerelay3", "0" );
    ////////////////////////////////////////////////////
  }
  //////////////////////////////////////////////////////////////////
  //Halaman
  if (Firebase.getString(firebaseData, "/statusrelay1"))
  {
    if (firebaseData.dataType() == "string")
    {
      String cekstatushalaman = firebaseData.stringData();
      String years = String(currentYear);
      if (cekstatushalaman == "HIDUP")
      {
        int d = (millis() / 1000);
        Serial.println(d);
        // EEPROM.put(data1, d);
        // data1 += sizeof(int);
        // EEPROM.put(data1, d);
        // int a;
        // EEPROM.commit();
        String i = String(d);
        Firebase.set(firebaseData, "timerelay1", i);
        //Firebase.setString(firebaseData, years+'/'+currentMonthName+'/'+monthDay+'/'+"Waktu Mulai", "COba masuk enggak");
      }
      ///////
      if (Firebase.getString(firebaseData, "/timerelay1")) {
        if (firebaseData.dataType() == "string") {
          String i = firebaseData.stringData();
          Firebase.setString(firebaseData, years + '/' + currentMonthName + '/' + monthDay + '/' + "PenggunaanRelay1", i);
          if (Firebase.getString(firebaseData, "/dayahalaman")) {
          }
        }
        ///////////////////////
        if (cekstatushalaman == "MATI")
        {

          /* if (Firebase.getString(firebaseData, "/timerelay1")){
             if (firebaseData.dataType() == "string")
             {
               String i = firebaseData.stringData();
               int a = i.toInt();
               int b;
               EEPROM.get(data1, b);
               int v = a + b;
               String sends = String(v);
               EEPROM.put(saverelay1, v);
               saverelay1 += sizeof(int);
               EEPROM.put(saverelay1, v);
            //    Firebase.set(firebaseData,"timerelay1", sends);
            //   Firebase.set(firebaseData, years+'/'+currentMonthName+'/'+monthDay+'/'+"PenggunaanRelay1", sends);
               Serial.println(sends);
               EEPROM.commit();
             }
            }*/
        }
      }
    }
    //////////////////////////////////////////////////////////////////
    //RuangTamu
    if (Firebase.getString(firebaseData, "/statusrelay2"))
    {
      if (firebaseData.dataType() == "string")
      {
        String cekstatushalaman = firebaseData.stringData();
        String years = String(currentYear);
        if (cekstatushalaman == "HIDUP")
        {
          int d = (millis() / 1000);
          Serial.println(d);
          // EEPROM.put(data1, d);
          // data1 += sizeof(int);
          // EEPROM.put(data1, d);
          // int a;
          // EEPROM.commit();
          String i = String(d);
          Firebase.set(firebaseData, "timerelay2", i);
          //Firebase.setString(firebaseData, years+'/'+currentMonthName+'/'+monthDay+'/'+"Waktu Mulai", "COba masuk enggak");
        }
        ///////
        if (Firebase.getString(firebaseData, "/timerelay2")) {
          if (firebaseData.dataType() == "string") {
            String i = firebaseData.stringData();
            Firebase.setString(firebaseData, years + '/' + currentMonthName + '/' + monthDay + '/' + "PenggunaanRelay2", i);
          }
        }
        ///////////////////////
        if (cekstatushalaman == "MATI")
        {

          /* if (Firebase.getString(firebaseData, "/timerelay1")){
             if (firebaseData.dataType() == "string")
             {
               String i = firebaseData.stringData();
               int a = i.toInt();
               int b;
               EEPROM.get(data1, b);
               int v = a + b;
               String sends = String(v);
               EEPROM.put(saverelay1, v);
               saverelay1 += sizeof(int);
               EEPROM.put(saverelay1, v);
            //    Firebase.set(firebaseData,"timerelay1", sends);
            //   Firebase.set(firebaseData, years+'/'+currentMonthName+'/'+monthDay+'/'+"PenggunaanRelay1", sends);
               Serial.println(sends);
               EEPROM.commit();
             }
            }*/
        }
      }
    }
    //////////////////////////////////////////////////////////////////
    //Pump
    if (Firebase.getString(firebaseData, "/statusrelay3"))
    {
      if (firebaseData.dataType() == "string")
      {
        String cekstatushalaman = firebaseData.stringData();
        String years = String(currentYear);
        if (cekstatushalaman == "HIDUP")
        {
          int d = (millis() / 1000);
          Serial.println(d);
          // EEPROM.put(data1, d);
          // data1 += sizeof(int);
          // EEPROM.put(data1, d);
          // int a;
          // EEPROM.commit();
          String i = String(d);
          Firebase.set(firebaseData, "timerelay3", i);
          //Firebase.setString(firebaseData, years+'/'+currentMonthName+'/'+monthDay+'/'+"Waktu Mulai", "COba masuk enggak");
        }
        ///////
        if (Firebase.getString(firebaseData, "/timerelay3")) {
          if (firebaseData.dataType() == "string") {
            String i = firebaseData.stringData();
            Firebase.setString(firebaseData, years + '/' + currentMonthName + '/' + monthDay + '/' + "PenggunaanRelay3", i);
          }
        }
        ///////////////////////
        if (cekstatushalaman == "MATI")
        {

          /* if (Firebase.getString(firebaseData, "/timerelay1")){
             if (firebaseData.dataType() == "string")
             {
               String i = firebaseData.stringData();
               int a = i.toInt();
               int b;
               EEPROM.get(data1, b);
               int v = a + b;
               String sends = String(v);
               EEPROM.put(saverelay1, v);
               saverelay1 += sizeof(int);
               EEPROM.put(saverelay1, v);
            //    Firebase.set(firebaseData,"timerelay1", sends);
            //   Firebase.set(firebaseData, years+'/'+currentMonthName+'/'+monthDay+'/'+"PenggunaanRelay1", sends);
               Serial.println(sends);
               EEPROM.commit();
             }
            }*/
        }
      }
    }
  }
}
