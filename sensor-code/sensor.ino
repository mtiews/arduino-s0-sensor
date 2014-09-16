// #########################################################################
// # Copyright 2014 Marcel Tiews marcel.tiews@gmail.com
// #########################################################################
// #
// # This sketch is free software: you can redistribute it and/or modify
// # it under the terms of the GNU General Public License as published by
// # the Free Software Foundation, either version 3 of the License, or
// # (at your option) any later version.
// #
// # This plugin is distributed in the hope that it will be useful,
// # but WITHOUT ANY WARRANTY; without even the implied warranty of
// # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// # GNU General Public License for more details.
// #
// # You should have received a copy of the GNU General Public License
// # along with this plugin. If not, see <http://www.gnu.org/licenses/>.
// #########################################################################

// #######################################################################
// Sensor with 4 S0 inputs incl. temp/hum
// #######################################################################

#include "DHT.h"
#include <PinChangeInt.h>

// loop delay, values will be send via serial port in a loop
#define LOOP_DELAY 15000

// DHT SETUP
#define DHTPIN 6 // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// Info led
const int STATUS_LED_PIN = 7;

// S0 configuration
const int S0_1_IMPULSE_PIN = 9;
const int S0_1_IMPULSES_PER_KWH = 1000;
const int S0_2_IMPULSE_PIN = 10;
const int S0_2_IMPULSES_PER_KWH = 1000;
const int S0_3_IMPULSE_PIN = 11;
const int S0_3_IMPULSES_PER_KWH = 1000;
const int S0_4_IMPULSE_PIN = 12;
const int S0_4_IMPULSES_PER_KWH = 1000;

struct status_struct {
  float temperature;
  float humidity;
  unsigned long s0_1_Count;
  unsigned long s0_1_ImpulseLength;
  unsigned long s0_1_LastTimestamp;
  unsigned long s0_2_Count;
  unsigned long s0_2_ImpulseLength;
  unsigned long s0_2_LastTimestamp;
  unsigned long s0_3_Count;
  unsigned long s0_3_ImpulseLength;
  unsigned long s0_3_LastTimestamp;
  unsigned long s0_4_Count;
  unsigned long s0_4_ImpulseLength;
  unsigned long s0_4_LastTimestamp;
} volatile my_status;

void setup() {
  pinMode(STATUS_LED_PIN,OUTPUT);
  pinMode(S0_1_IMPULSE_PIN, INPUT_PULLUP);      
  pinMode(S0_2_IMPULSE_PIN, INPUT_PULLUP);   
  pinMode(S0_3_IMPULSE_PIN, INPUT_PULLUP);   
  pinMode(S0_4_IMPULSE_PIN, INPUT_PULLUP);   

  PCintPort::attachInterrupt(S0_1_IMPULSE_PIN, &s0Impulse1Event, FALLING);
  PCintPort::attachInterrupt(S0_2_IMPULSE_PIN, &s0Impulse2Event, FALLING);
  PCintPort::attachInterrupt(S0_3_IMPULSE_PIN, &s0Impulse3Event, FALLING);
  PCintPort::attachInterrupt(S0_4_IMPULSE_PIN, &s0Impulse4Event, FALLING);

  Serial.begin(115200);
  dht.begin();
}

void loop() {
  my_status.humidity = dht.readHumidity();
  my_status.temperature = dht.readTemperature();
    
  submitStatus();
  delay(LOOP_DELAY);
}

void s0Impulse1Event()
{
  unsigned long current = millis();
  if(my_status.s0_1_LastTimestamp > 0)
    my_status.s0_1_ImpulseLength = current - my_status.s0_1_LastTimestamp;
  my_status.s0_1_LastTimestamp = current;
  my_status.s0_1_Count++;
}

void s0Impulse2Event()
{
  unsigned long current = millis();
  if(my_status.s0_2_LastTimestamp > 0)
    my_status.s0_2_ImpulseLength = current - my_status.s0_2_LastTimestamp;
  my_status.s0_2_LastTimestamp = current;
  my_status.s0_2_Count++;
}

void s0Impulse3Event()
{
  unsigned long current = millis();
  if(my_status.s0_3_LastTimestamp > 0)
    my_status.s0_3_ImpulseLength = current - my_status.s0_3_LastTimestamp;
  my_status.s0_3_LastTimestamp = current;
  my_status.s0_3_Count++;
}

void s0Impulse4Event()
{
  unsigned long current = millis();
  if(my_status.s0_4_LastTimestamp > 0)
    my_status.s0_4_ImpulseLength = current - my_status.s0_4_LastTimestamp;
  my_status.s0_4_LastTimestamp = current;
  my_status.s0_4_Count++;
}

unsigned long calcActualImpulseLength(unsigned long last_length, unsigned long last_impulse_timestamp)
{
  if(last_impulse_timestamp == 0)
    return last_length;
  unsigned long current_length = millis() - last_impulse_timestamp;
  return max(current_length, last_length);
}

bool submitStatus()
{
  // Get current impulse counters and reset them
  cli();
  unsigned long count1 = my_status.s0_1_Count;
  my_status.s0_1_Count = 0;
  unsigned long count2 = my_status.s0_2_Count;
  my_status.s0_2_Count = 0;
  unsigned long count3 = my_status.s0_3_Count;
  my_status.s0_3_Count = 0;
  unsigned long count4 = my_status.s0_4_Count;
  my_status.s0_4_Count = 0;
  sei();

  unsigned long imp = 0;
  
  // Temp / Humidity
  Serial.print("TH#8eff16c9-b53b-4d08-8051-ee3737fd0332#TEMP:");
  Serial.print(my_status.temperature);
  Serial.print("#HUM:");
  Serial.println(my_status.humidity);
  
  // S0_1
  Serial.print("S0#cceed198-81c9-4f62-9592-374c5fc8611c#WH:");
  Serial.print(count1 * 1000 / S0_1_IMPULSES_PER_KWH);
  Serial.print("#W:");
  imp = calcActualImpulseLength(my_status.s0_1_ImpulseLength,my_status.s0_1_LastTimestamp);
  if(imp > 0)
    Serial.print(3600000000 / imp / S0_1_IMPULSES_PER_KWH);
  else
    Serial.print("0");
  Serial.print("#Impulses:");
  Serial.println(count1);
  
  // S0_2
  Serial.print("S0#18c0055f-68d3-4749-b335-8b029cf1cf16#WH:");
  Serial.print(count2 * 1000 / S0_2_IMPULSES_PER_KWH);
  Serial.print("#W:");
  imp = calcActualImpulseLength(my_status.s0_2_ImpulseLength,my_status.s0_2_LastTimestamp);
  if(imp > 0)
    Serial.print(3600000000 / imp / S0_2_IMPULSES_PER_KWH);
  else
    Serial.print("0");
  Serial.print("#Impulses:");
  Serial.println(count2);
  
  // S0_3
  Serial.print("S0#b9bc7486-d28b-4e3c-9146-0826599a11f5#WH:");
  Serial.print(count3 * 1000 / S0_3_IMPULSES_PER_KWH);
  Serial.print("#W:");
  imp = calcActualImpulseLength(my_status.s0_3_ImpulseLength,my_status.s0_3_LastTimestamp);
  if(imp > 0)
    Serial.print(3600000000 / imp / S0_3_IMPULSES_PER_KWH);
  else
    Serial.print("0");
  Serial.print("#Impulses:");
  Serial.println(count3);
  
  // S0_4
  Serial.print("S0#f8958335-dfce-4978-be01-131f646cb9e8#WH:");
  Serial.print(count4 * 1000 / S0_4_IMPULSES_PER_KWH);
  Serial.print("#W:");
  imp = calcActualImpulseLength(my_status.s0_4_ImpulseLength,my_status.s0_4_LastTimestamp);
  if(imp > 0)
    Serial.print(3600000000 / imp / S0_4_IMPULSES_PER_KWH);
  else
    Serial.print("0");
  Serial.print("#Impulses:");
  Serial.println(count4);
  
  blinkLed();
}

void blinkLed()
{
  digitalWrite(STATUS_LED_PIN,HIGH);
  delay(100);
  digitalWrite(STATUS_LED_PIN,LOW);
  delay(100);
  digitalWrite(STATUS_LED_PIN,HIGH);
  delay(100);
  digitalWrite(STATUS_LED_PIN,LOW);
}

