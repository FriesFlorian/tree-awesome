#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,1,203 };

//pinout
static int onpins[] = {0, 4, 6};
static int offpins[] = {0, 5, 7};

// state of the power outlets (true = on, false = off)
boolean postates[] = {false, false, false};

// current mode
int mode = 0;

// switch counter to avoid false positive
int switchcounter = 0;

static char* getState(int no)
{
  if(postates[no])  return "true";
  return "false";
}

static char* getMode()
{
  if(mode == 0)  return "normal";
  if(mode == 1)  return "switch";
  if(mode == 2)  return "light";
}


static char* getSwitchState()
{
  if(digitalRead(11) == HIGH)  return "true";
  else  return "false";
}


void setPO(int no, boolean state)
{
  if(postates[no] != state)
  {
    if(state)
    {
      digitalWrite(onpins[no], LOW);
      delay(200);
      digitalWrite(onpins[no], HIGH);
      postates[no] = true;
    }
    else
    {
      digitalWrite(offpins[no], LOW);
      delay(200);
      digitalWrite(offpins[no], HIGH);
      postates[no] = false;
    }
  }
}

void allLow()
{
  digitalWrite(onpins[1], LOW);
  digitalWrite(offpins[1], LOW);
  digitalWrite(onpins[2], LOW);
  digitalWrite(offpins[2], LOW);
}


byte Ethernet::buffer[500];
BufferFiller bfill;

void setup () {
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  ether.staticSetup(myip);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  
  pinMode(11, INPUT); // Reed switch
}

static word getData() {
  long t = millis() / 1000;
  word h = t / 3600;
  byte m = (t / 60) % 60;
  byte s = t % 60;
  
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "{\"uptime\":\"$D$D:$D$D:$D$D\", \"mode\":\"$S\", \"switch\":$S, \"po1\":$S, \"po2\":$S}"),
      h/10, h%10, m/10, m%10, s/10, s%10, getMode(), getSwitchState(), getState(1), getState(2));
  return bfill.position();
}

void loop () {
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);

  // check what to do according to the current mode
  switch(mode)
  {
    case 0: //normal, do nothing special
      break;
    case 1: //switch (reed sensor, drink)
      if(digitalRead(11) == HIGH)
      {
        if(switchcounter >= 1000)  setPO(1, true); // the counter makes sure that the switch was triggered long enough
        else  switchcounter++;
      }
      else  switchcounter = 0;
      
      break;
    case 2: //Light sensor
      if(analogRead(0) < 10)  setPO(1, true); // if the level of light is under '10', we power the first power outlet
      else  setPO(1, false);
      break;
    case 3: //reset both power outlets
      setPO(1, false);
      setPO(2, false);
      mode = 0; //and switch back to normal mode
      break;
  }
  
  
  if(analogRead(0));
    
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if (pos) // check if valid tcp data is received
  {
    bfill = ether.tcpOffset();
    char* data = (char *) Ethernet::buffer + pos;
    
    // Analyse the request url
    if  (strncmp("GET /po1", data, 8) == 0)
      setPO(1, !postates[1]);
    else if  (strncmp("GET /po2", data, 8) == 0)
      setPO(2, !postates[2]);
    else if  (strncmp("GET /normal", data, 11) == 0) // normal mode
      mode = 0;
    else if  (strncmp("GET /switch", data, 11) == 0) // switch mode
      mode = 1;
    else if  (strncmp("GET /light", data, 10) == 0) // light sensor mode
      mode = 2;
    else if  (strncmp("GET /reset", data, 10) == 0) // reset mode
      mode = 3;
      
    //display the data
    ether.httpServerReply(getData());
  } 
  
}
