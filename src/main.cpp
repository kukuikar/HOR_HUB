#include <Arduino.h>
#include <GyverPortal.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include "WIFI_AP.h"

GyverPortal ui(&LittleFS);

////////////////////////////////////
// Buffer Size for UDP Packets
////////////////////////////////////
const uint8_t BUFFER_SIZE = 32;

////////////////////////////////////
// Prototypes
////////////////////////////////////
void startup();
void build();
void action();
void FormatBuffer(char *buffer, int n, ...);
void UDP_transmit(IPAddress udpIP, char *tr);
void FillClientsStruct(char *packet);
void checkClients(char *packet);

struct ClientsIp
{
    IPAddress BridgeIP, SpreaderIP, CranesIP, LiftsIP;
    bool BridgeConnected, SpreaderConnected, CranesConnected, LiftsConnected;
};
ClientsIp AP_Clients;

////////////////////////////////////
// Default Motors Rate
////////////////////////////////////
int Rate = 50;
////////////////////////////////////

WiFiUDP UDP;

////////////////////////////////////
// Bool Flags
////////////////////////////////////
bool Bbl_Flag = false; // флаг движения моста влево
bool Bbr_Flag = false; // флаг движения моста вправо
bool Btu_Flag = false; // флаг движения тележки вверх
bool Btd_Flag = false; // флаг движения тележки вниз
bool Bwu_Flag = false; // флаг движения лебедки вверх
bool Bwd_Flag = false; // флаг движения лебедки вниз

bool Lu_Flag = false; // флаг движения моста влево
bool Ld_Flag = false; // флаг движения моста вправо

bool Stt_Flag = false; // флаг телескопов сдвинуть
bool Std_Flag = false; // флаг телескопов раздвинуть
bool Sr1_Flag = false; // поворот против часовой
bool Sr2_Flag = false; // поворот против часовой
bool Stl_Flag = false; // замки закрыты
bool Stu_Flag = false; // замки открыты

////////////////////////////////////
// UI Const
////////////////////////////////////
const uint8_t iconH = 80;
const uint8_t buttonW = 90;
const char bW[5] = "80px";

void setup()
{
  startup();

  // подключаем конструктор и запускаем
  ui.attachBuild(build);
  ui.attach(action);
  ui.start();
}

void build()
{
  GP.BUILD_BEGIN();
  //GP.setTimeout(2000);
  GP.THEME(GP_DARK);
  GP.TITLE("MKD HORIZONE");
  //GP.ICON_SUPPORT();

  //GP.IMAGE("/Bbr.png");  GP.BREAK();

  GP.NAV_TABS("Bridge,Spreader,Lift");
  GP.SLIDER("Rate", Rate);

  //////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////
  GP.NAV_BLOCK_BEGIN();

  GP.TABLE_BEGIN("45%,45%", GP_ALS(GP_CENTER, GP_CENTER, GP_CENTER));

  GP.TR();
  GP.TD(GP_CENTER, 2);
  GP.BUTTON("Btu", GP.ICON_FILE("/Btu.svg", iconH), "Btu", GP_ORANGE, bW);
  GP.TD(GP_CENTER);
  GP.BUTTON("Bwu", GP.ICON_FILE("/Bwu.svg", iconH), "Bwu", GP_ORANGE, bW);

  GP.TR();
  GP.TD(GP_CENTER);
  GP.BUTTON("Bbl", GP.ICON_FILE("/Bbl.svg", iconH), "Bbl", GP_ORANGE, bW);
  GP.TD(GP_CENTER);
  GP.BUTTON("Bbr", GP.ICON_FILE("/Bbr.svg", iconH), "Bbr", GP_ORANGE, bW);
  GP.TD(GP_CENTER);

  GP.TR();
  GP.TD(GP_CENTER, 2);
  GP.BUTTON("Btd", GP.ICON_FILE("/Btd.svg", iconH), "Btd", GP_ORANGE, bW);
  GP.TD(GP_CENTER, 2);
  GP.BUTTON("Bwd", GP.ICON_FILE("/Bwd.svg", iconH), "Bwd", GP_ORANGE, bW);

  GP.TABLE_END();

  GP.NAV_BLOCK_END();

  //////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////
  GP.NAV_BLOCK_BEGIN();

  GP.TABLE_BEGIN("25%,25%,15%,25%", GP_ALS(GP_CENTER, GP_CENTER, GP_CENTER));

  GP.TR();
  GP.TD();
  GP.BUTTON("Stt", GP.ICON_FILE("/Stt.svg", iconH), "Stt", GP_ORANGE, bW); 
  GP.TD();
  GP.BUTTON("Sr1", GP.ICON_FILE("/Sr1.svg", iconH), "Sr1", GP_ORANGE, bW);
  GP.TD();
  GP.TD();
  GP.BUTTON("Stl", GP.ICON_FILE("/Stl.svg", iconH), "Stl", GP_ORANGE, bW);

  GP.TR();
  GP.TD();
  GP.BUTTON("Std", GP.ICON_FILE("/Std.svg", iconH), "Std", GP_ORANGE, bW);
  GP.TD();
  GP.BUTTON("Sr2", GP.ICON_FILE("/Sr2.svg", iconH), "Sr2", GP_ORANGE, bW);
  GP.TD();
  GP.TD();
  GP.BUTTON("Stu", GP.ICON_FILE("/Stu.svg", iconH), "Stu", GP_ORANGE, bW); 

  GP.TABLE_END();

  GP.NAV_BLOCK_END();

  //////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////
  GP.NAV_BLOCK_BEGIN();

  GP.BUTTON("Lu", GP.ICON_FILE("/Lu.svg", iconH), "Lu", GP_ORANGE, bW);
  GP.BREAK();
  GP.BUTTON("Ld", GP.ICON_FILE("/Ld.svg", iconH), "Ld", GP_ORANGE, bW);

  GP.NAV_BLOCK_END();

  //GP.FILE_MANAGER(&LittleFS);

  GP.BUILD_END();
}

void action()
{
  if (ui.hold())
  {
    if(ui.clickDown("Bbl")) Bbl_Flag = true;
    if(ui.clickUp("Bbl"))   Bbl_Flag = false;

    if(ui.clickDown("Bbr")) Bbr_Flag = true;
    if(ui.clickUp("Bbr"))   Bbr_Flag = false;

    if(ui.clickDown("Btu")) Btu_Flag = true;
    if(ui.clickUp("Btu"))   Btu_Flag = false;

    if(ui.clickDown("Btd")) Btd_Flag = true;
    if(ui.clickUp("Btd"))   Btd_Flag = false;

    if(ui.clickDown("Bwu")) Bwu_Flag = true;
    if(ui.clickUp("Bwu"))   Bwu_Flag = false;

    if(ui.clickDown("Bwd")) Bwd_Flag = true;
    if(ui.clickUp("Bwd"))   Bwd_Flag = false;

    ///////////////////////////////////////////////////////////
    if(ui.clickDown("Stt")) Stt_Flag = true;
    if(ui.clickUp("Stt"))   Stt_Flag = false;

    if(ui.clickDown("Std")) Std_Flag = true;
    if(ui.clickUp("Std"))   Std_Flag = false;

    if(ui.clickDown("Sr1")) Sr1_Flag = true;
    if(ui.clickUp("Sr1"))   Sr1_Flag = false;

    if(ui.clickDown("Sr2")) Sr2_Flag = true;
    if(ui.clickUp("Sr2"))   Sr2_Flag = false;

    if(ui.clickDown("Stl")) Stl_Flag = true;
    if(ui.clickUp("Stl"))   Stl_Flag = false;

    if(ui.clickDown("Stu")) Stu_Flag = true;
    if(ui.clickUp("Stu"))   Stu_Flag = false;


    ///////////////////////////////////////////////////////////
    if(ui.clickDown("Lu")) Lu_Flag = true;
    if(ui.clickUp("Lu"))   Lu_Flag = false;

    if(ui.clickDown("Ld")) Ld_Flag = true;
    if(ui.clickUp("Ld"))   Ld_Flag = false;
  }

  if (ui.click("Rate"))
  {
    Rate = ui.getInt("Rate");
  }
}

void loop()
{
  ui.tick();

  int packetSize = UDP.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet! Size: ");
    Serial.println(packetSize); 
    char packet[BUFFER_SIZE] = "";
    int len = UDP.read(packet, BUFFER_SIZE);
    if (len > 0)
    {
      packet[len] = '\0';
    }

    FillClientsStruct(packet);
  }

  ///////////////////////////////////////////////////
  // Обработка флагов кнопок
  ///////////////////////////////////////////////////
  static uint32_t tmr = millis();
  static uint8_t tmr_interval = 20;
  if (millis() - tmr > tmr_interval)
  {
    tmr = millis();

    vTaskDelay(1);

    uint8_t vel = 0;
    if (Rate != 0)
      vel = map(Rate, 0, 100, 0, 255);

    char buf[BUFFER_SIZE] = "";
    ///////////////////////////////////////////////////
    // BRIDGE
    ///////////////////////////////////////////////////
    if (Bbl_Flag || Bbr_Flag || Btu_Flag || Btd_Flag || Bwu_Flag || Bwd_Flag)
    {
      if (Bbl_Flag)
      {
        FormatBuffer(buf, 5, 0, 1, -vel, 0, 0);
        Serial.print("Drive bridge left PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.BridgeIP, buf);
      }

      if (Bbr_Flag)
      {
        FormatBuffer(buf, 5, 0, 1, vel, 0, 0);
        Serial.print("Drive bridge right PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.BridgeIP, buf);
      }

      if (Btu_Flag)
      {
        FormatBuffer(buf, 5, 0, 1, 0, vel, 0);
        Serial.print("Drive trolley UP PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.BridgeIP, buf);
      }

      if (Btd_Flag)
      {
        FormatBuffer(buf, 5, 0, 1, 0, -vel, 0);
        Serial.print("Drive trolley DOWN PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.BridgeIP, buf);
      }

      if (Bwu_Flag)
      {
        FormatBuffer(buf, 5, 0, 1, 0, 0, vel);
        Serial.print("Drive winch UP PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.BridgeIP, buf);
      }

      if (Bwd_Flag)
      {
        FormatBuffer(buf, 5, 0, 1, 0, 0, -vel);
        Serial.print("Drive winch DOWN PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.BridgeIP, buf);
      }
    }

    ///////////////////////////////////////////////////
    // SPREADER
    ///////////////////////////////////////////////////
    else if (Stt_Flag || Std_Flag || Sr1_Flag || Sr2_Flag || Stl_Flag || Stu_Flag)
    {
      if (Stt_Flag)
      {
        FormatBuffer(buf, 5, 1, 1, 0, -vel, 0);
        Serial.print("Drive telescopes INSIDE PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.SpreaderIP, buf);
      }

      if (Std_Flag)
      {
        FormatBuffer(buf, 5, 1, 1, 0, vel, 0);
        Serial.print("Drive telescopes OUTSIDE PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.SpreaderIP, buf);
      }

      if (Sr1_Flag)
      {
        FormatBuffer(buf, 5, 1, 1, -vel, 0, 0);
        Serial.print("Rotate tower CCW PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.SpreaderIP, buf);
      }

      if (Sr2_Flag)
      {
        FormatBuffer(buf, 5, 1, 1, vel, 0, 0);
        Serial.print("Rotate tower CW PWM Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.SpreaderIP, buf);
      }

      if (Stl_Flag)
      {
        int ang = 0;
        if (vel != 0)
          ang = map(vel, 0, 255, 90, 180);

        FormatBuffer(buf, 5, 1, 1, 0, 0, ang);
        Serial.print("Lock twistlocks ANG Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.SpreaderIP, buf);
      }

      if (Stu_Flag)
      {
        int ang = 0;
        if (vel != 0)
          ang = map(vel, 0, 255, 90, 0);

        FormatBuffer(buf, 5, 1, 1, 0, 0, ang);
        Serial.print("Unlock twistlocks ANG Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.SpreaderIP, buf);
      }
    }
    ///////////////////////////////////////////////////
    // LIFTING
    ///////////////////////////////////////////////////
    else if (Lu_Flag || Ld_Flag)
    {
      if (Lu_Flag)
      {
        int ang = 0;
        if (vel != 0)
          ang = map(vel, 0, 255, 90, 180);

        FormatBuffer(buf, 4, 3, 1, ang, 6);
        Serial.print("Lifting UP ANG Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.LiftsIP, buf);
      }

      if (Ld_Flag)
      {
        int ang = 0;
        if (vel != 0)
          ang = map(vel, 0, 255, 90, 0);

        FormatBuffer(buf, 4, 3, 1, ang, 6);
        Serial.print("Lifting DOWN ANG Buffer: ");
        Serial.println(buf);
        UDP_transmit(AP_Clients.LiftsIP, buf);
      }
    }
  }
  ///////////////////////////////////////////////////
}

void startup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(__SSID, __PSWD);
  Serial.println(WiFi.softAPIP());

  if (!LittleFS.begin())
    Serial.println("LittleFS Error");
  else
    Serial.println("LittleFS Started");

  UDP.begin(__PORT);
}

void FormatBuffer(char *buffer, int n, ...)
{
  va_list factor;
  va_start(factor, n);
  for (int i = 0; i < n; i++)
  {
    itoa(va_arg(factor, int), buffer + strlen(buffer), DEC);
    strcat(buffer, ",");
  }
  va_end(factor);
  buffer[strlen(buffer) - 1] = ';';
}

void UDP_transmit(IPAddress udpIP, char *buffer)
{
  UDP.beginPacket(udpIP, __PORT);
  int n = UDP.printf(buffer);
  Serial.print("Sended ");
  Serial.println(n);
  UDP.endPacket();
}

void FillClientsStruct(char packet[BUFFER_SIZE])
{
  char Action = packet[0];
  // Serial.println("Finding");
  if (Action == '_')
  {
    char Device = packet[1];
    switch (Device)
    {
    case 'A': // BRIDGE
      AP_Clients.BridgeIP = UDP.remoteIP();
      AP_Clients.BridgeConnected = true;
      Serial.print("Bridge ");
      Serial.print("connected, IP ");
      Serial.println(AP_Clients.BridgeIP);
      break;
    case 'B': // SPREADER
      AP_Clients.SpreaderIP = UDP.remoteIP();
      AP_Clients.SpreaderConnected = true;
      Serial.print("Spreader ");
      Serial.print("connected, IP ");
      Serial.println(AP_Clients.SpreaderIP);
      break;
    case 'C': // CRANES
      AP_Clients.CranesIP = UDP.remoteIP();
      AP_Clients.CranesConnected = true;
      Serial.print("Cranes ");
      Serial.print("connected, IP ");
      Serial.println(AP_Clients.CranesIP);
      break;
    case 'D': // LIFT
      AP_Clients.LiftsIP = UDP.remoteIP();
      AP_Clients.LiftsConnected = true;
      Serial.print("Lifts ");
      Serial.print("connected, IP ");
      Serial.println(AP_Clients.LiftsIP);
      break;
    }
  }
}