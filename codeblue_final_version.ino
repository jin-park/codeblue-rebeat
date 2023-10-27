#include <Wire.h>
#include <SPI.h>

#include "SparkFun_BNO080_Arduino_Library.h"  // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
BNO080 imu;

#include <Adafruit_GFX.h>
#include "MCUFRIEND_kbv.h"
MCUFRIEND_kbv tft;

#define LOWFLASH (defined(__AVR_ATmega328P__) && defined(MCUFRIEND_KBV_H_))
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define GREY 0x8410
#define ORANGE 0xE880
#define DARKGRAY 0xBDD7

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 7;
bool serialOn = false;
long color = 0x674A;

void setup() {
  if (serialOn) {

    Serial.println();
    Serial.println(F("BNO080 Read Example"));
  }
  Serial.begin(115200);

  uint16_t ID = tft.readID();

  tft.begin(ID);
  tft.fillScreen(BLACK);

  Wire.begin();

  imu.begin();

  Wire.setClock(400000);  

  imu.enableLinearAccelerometer(interval); 
  if (serialOn) {

    Serial.println(F("Linear Accelerometer enabled"));
    Serial.println(F("Output in form x, y, z, in m/s^2"));
  }
  previousMillis = millis();

  tft.fillRect(44, 240 + 105, 45, 1, WHITE);
  tft.setCursor(45, 385);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print(F("DEPTH"));

  tft.fillRect(145, 90, 145, 70, 0xFFFF);
  tft.fillRect(150, 95, 135, 60, 0x0000);

  tft.setCursor(165, 115);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);
  tft.print(F("COUNT: "));

  tft.fillRect(145, 190, 145, 200, 0xFFFF);
  tft.fillRect(150, 195, 135, 190, 0x0000);
  tft.setCursor(158, 205);
  tft.setTextColor(DARKGRAY, BLACK);
  tft.setTextSize(3);
  tft.print(F("RELEASE"));

  tft.setCursor(158, 255);
  tft.print(F("SLOWER"));

  tft.setCursor(158, 305);

  tft.print(F("FASTER"));

  tft.setCursor(158, 355);
  tft.print(F("DEEPER"));

  tft.fillRect(44, 140, 45, 5, WHITE);
  tft.fillRect(44, 139, 45, 1, WHITE);


}

void reset_bar() {
  tft.fillRect(44, 140, 45, 214, BLACK);
}

unsigned long count = 0;
double displacement = 0, prev_disp = 0, velocity = 0, prev_z = 0, prev_vel = 0;
unsigned long prevsec = 0, prevupdate = 0;
int acc_count = 0, adj_acc_count = 0;
bool reset = false;

double moving_avg[4]{ 0.0, 0.0, 0.0, 0.0 };
double avg = 0.0;
bool direction = true;
long barpercent = 0, barprev = 0, tempocnt = 0;
unsigned long cnt = 0;
bool disp_updated = false;
unsigned long now;

bool once = false, once2 = false;
unsigned long timecnt = 0, hour = 0, minute = 0, second = 0, inactivitycnt = 0;
bool active = true, goingdown = false, goingup = false;
double last_ten_depth[10]{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
long last_ten_freq[10]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
double deepest = 0.0, avg_depth, avg_freq;
int depth_percent, final_depth = 0, final_freq = 0;
double summary_depth, summary_freq;
long cpr_end_count = 0;
int state = 4, upd;
bool lock = false, vel_reset = false, overzero = false, rst = false, lock_once = false;

unsigned long last_integrated = 0, deep_time = 0, prev_deep_time = 0;

bool pressure_reset_once, started = false, reset_once;
int pressure_pin = 7;
int pressure = 0, p_pressure, min_pressure = 2000;
float x, y, z;
unsigned long diff;
String time_str;

int piezo_pin = 30;

void update_time() {
  tft.setCursor(70, 40);
  hour = 0;
  minute = 0;
  second = 0;
  second = (unsigned int)timecnt / 2;

  if ((unsigned int)second / 3600 > 0) {
    hour = (unsigned int)second / 3600;
  }
  minute = (unsigned int)((second % 3600) / 60);
  second = (unsigned int)(second % 60);
  if (hour > 0) {
    tft.print(hour);
    tft.print(F(":"));
    if (minute < 10) {
      tft.print(0);
    }
  }
  tft.print(minute);
  tft.print(F(":"));
  time_str = time_str + minute + ":";
  if (second < 10) {
    tft.print(0);
  }
  tft.print(second);
}

void start_display() {
  tft.fillRect(44, 240 + 105, 45, 1, WHITE);
  tft.setCursor(45, 385); 
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print(F("DEPTH"));

  tft.fillRect(145, 90, 145, 70, 0xFFFF);
  tft.fillRect(150, 95, 135, 60, 0x0000);

  tft.setCursor(165, 115);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);
  tft.print(F("COUNT: "));

  tft.fillRect(145, 190, 145, 200, 0xFFFF);
  tft.fillRect(150, 195, 135, 190, 0x0000);
  tft.setCursor(158, 205);
  tft.setTextColor(DARKGRAY, BLACK);
  tft.setTextSize(3);
  tft.print(F("RELEASE"));

  tft.setCursor(158, 255);
  tft.print(F("SLOWER"));

  tft.setCursor(158, 305);

  tft.print(F("FASTER"));

  tft.setCursor(158, 355);
  tft.print(F("DEEPER"));

  tft.fillRect(44, 140, 45, 5, WHITE);
  tft.fillRect(44, 139, 45, 1, WHITE);
}




bool warnings[4] = { false, false, false, false };          // RELEASE, SLOWER, FASTER, DEEPER
bool current_warnings[4] = { false, false, false, false };  // ACTUAL DISPLAY
unsigned long warnings_count[4] = { 0, 0, 0, 0 };
unsigned long total_warnings_count[4] = { 0, 0, 0, 0 };
double bpm = 0;
unsigned long lock_count = 0;
unsigned long lock_diff = 0;
bool speed_warning_updated = false;
unsigned long current_freq = 0;
unsigned long last_pump_time = 0;
bool evaluation_display = false, evaluation_displayed = false;
bool started_display = true, midpoint_reset_once = false, release_updated = false;
float average_bpm = 0, average_depth = 0;
unsigned long n_bpm = 0, n_depth = 0, hardest = 0, softest = 0;
unsigned long last_few_pressures[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long start_time;
unsigned long evaluation_display_time = 0, below_threshold_count = 0;
bool pause_calculations = false;

void reset_all() {
  currentMillis = millis(); previousMillis = currentMillis;
  count = 0;
  displacement = 0, prev_disp = 0, velocity = 0, prev_z = 0, prev_vel = 0;
  prevsec = currentMillis, prevupdate = currentMillis;
  acc_count = 0, adj_acc_count = 0;
  reset = false;

  for (int i = 0; i < 4; i++) {
    moving_avg[i] = 0.0;
  }

  avg = 0.0;
  barpercent = 0, barprev = 0, tempocnt = 0;
  cnt = 0;
  disp_updated = false;

  once = false, once2 = false;
  timecnt = 0, hour = 0, minute = 0, second = 0, inactivitycnt = 0;
  active = false, goingdown = false, goingup = false;
  for (int i = 0; i < 10; i++) {
    last_ten_depth[i] = 1;
    last_ten_freq[i] = 0;
  }

  deepest = 0.0;
  depth_percent = 0, final_depth = 0, final_freq = 0;
  cpr_end_count = 0;
  lock = false, vel_reset = false, overzero = false, rst = false, lock_once = false;
  
  last_integrated = currentMillis, deep_time = currentMillis, prev_deep_time = currentMillis;

  pressure_reset_once = false, started = false, reset_once = false;
  pressure = 0, p_pressure = 0, min_pressure = 2000;
  diff = 0; release_updated = false; below_threshold_count = 0; pause_calculations = false;



  for (int i = 0; i < 4; i++) {
    warnings[i] = false;
    current_warnings[i] = false;
    warnings_count[i] = 0;
    total_warnings_count[i] = 0;
  }
  bpm = 0;
  lock_count = 0;
  lock_diff = 0;
  current_freq = 0;
  last_pump_time = currentMillis;

  midpoint_reset_once = false;
  average_bpm = 0, average_depth = 0;
  n_bpm = 0, n_depth = 0, hardest = 0, softest = 0;
  for (int i = 0; i < 10; i++) {
    last_few_pressures[i] = 0;
  }
}


unsigned long calc_diff() {
  unsigned long max_elem = 0, min_elem = 1000;
  for (int i = 0; i < 10; i++) {
    max_elem = max(max_elem, last_few_pressures[i]);
    min_elem = min(min_elem, last_few_pressures[i]);
  }
  return max_elem - min_elem;
}

double stdev() {
  double mean = 0;
  for (int i = 0; i < 10; i++) {
    mean += last_few_pressures[i];
  }
  mean /= 10;
  double std = 0;
  for (int i = 0; i < 10; i++) {
    std += (mean - last_few_pressures[i]) * (mean - last_few_pressures[i]);
  }
  std /= 10;
  return sqrt(std);
}

void evaluation() {
  tft.fillScreen(BLACK);

  tft.fillRect(158, 120, 6, 270, WHITE);
  
  tft.setCursor(105, 50);
  tft.setTextSize(3);
  tft.setTextColor(WHITE, BLACK);
  tft.print("RESULTS");

  tft.setCursor(30, 140);
  tft.setTextSize(2);
  tft.print("AVG BPM");

  tft.setCursor(30, 170);
  tft.setTextSize(3);
  int average_bpm_int = (int) average_bpm;
  tft.print(average_bpm_int);
  tft.print(" BPM");

  Serial.print("average bpm: ");
  Serial.println(average_bpm);
  Serial.print("average_bpm_int: ");
  Serial.println(average_bpm_int);

  tft.setCursor(30, 210);
  tft.setTextSize(2);
  tft.print("AVG DEPTH");
  
  tft.setCursor(30, 240);
  tft.setTextSize(3);
  tft.print(abs(average_depth * 100), 1);
  tft.print(" CM");
  Serial.print("average depth: ");
  Serial.println(average_depth);

  tft.setCursor(30, 280);
  tft.setTextSize(2);
  tft.print("DURATION");
  
  tft.setCursor(30, 310);
  tft.setTextSize(3);

  hour = 0;
  minute = 0;
  second = 0;
  second = (unsigned long)(last_pump_time - start_time)/1000;

  if ((unsigned long)second / 3600 > 0) {
    hour = (unsigned long)second / 3600;
  }
  minute = (unsigned long)((second % 3600) / 60);
  second = (unsigned long)(second % 60);
  if (hour > 0) {
    tft.print(hour);
    tft.print(F(":"));
    if (minute < 10) {
      tft.print(0);
    }
  }
  tft.print(minute);
  tft.print(F(":"));
  time_str = time_str + minute + ":";
  if (second < 10) {
    tft.print(0);
  }
  
  tft.print(second);
  tft.setCursor(180, 220);
  tft.setTextSize(2);
  tft.print("FINAL SCORE");
  
  
  
  long final_score = 0;
  float intermediate;
  if (average_depth < -0.05 && average_depth > -0.06) {
    final_score = 50;
  } 
  else {
    final_score = 50;
    intermediate = (float) abs(abs(average_depth) - 0.05) * 1000; 
    intermediate = (int) intermediate * 2;
    if (intermediate > 50) {
      intermediate = 50;
    } 
    if (intermediate < 0) {
      intermediate = 0;
    }
    final_score -= (int) intermediate; 
  }
  if (average_bpm <= 120 && average_bpm >= 100) { 
    final_score += 50;
  }
  else {
    final_score += 50;
    intermediate = ((int) abs(average_bpm - 100));
    if (intermediate > 50) {
      intermediate = 50;
    }
    if (intermediate < 0) {
      intermediate = 0;
    }
    final_score -= intermediate;
  }
  tft.setCursor(180, 250);
  tft.setTextSize(4);
  tft.print(final_score);
}

void loop() {
  now = millis();
  if (now - prevsec > (unsigned long)545) {
    if (started && active) {
      noTone(piezo_pin);
      tone(piezo_pin, 1000, 50);
    }
    prevsec = now;
  }

  if (now - prevupdate > (unsigned long)1000) {
    prevupdate = now;
  }
  
  if (!started) {
    pressure = analogRead(pressure_pin);
    if (pressure > 30) {
      if (!started_display) {

        reset_all();
        tft.fillScreen(BLACK);
        start_display();
        started_display = true;
      }
      started = true;
      start_time = millis();
      last_pump_time = start_time;
      now = start_time;
      active = true;
    }
  }

  if (!started && !started_display && (unsigned long) now - evaluation_display_time > (unsigned long) (180 * 1000)) {
    reset_all();
    tft.fillScreen(BLACK);
    start_display();
    started_display = true;
    active = false;
  }

  if (started && now - last_pump_time > (15 * 1000)) {
    evaluation_display = true;
    started_display = false;
    started = false;
    evaluation();
    evaluation_display_time = millis();
    active = false;
  }


  if (disp_updated) {
    barprev = barpercent;
    barpercent = (long)((displacement / 0.05) * 100);
    if (barpercent > 0)
      barpercent = 0;
    barpercent = abs(barpercent);
    if (barpercent > 99)
      barpercent = 99;
    if (barpercent != barprev) {
      if (abs(barpercent - barprev) > 2) {
        tft.fillRect(44, 140 + 2 * barprev, 45, 5, BLACK);
        tft.fillRect(44, 140 + 2*barpercent, 45, 5, WHITE);
      } else if (barpercent > barprev) {
        tft.fillRect(44, 140 + 2*barprev, 45, 2 * (barpercent - barprev), BLACK);
        tft.fillRect(44, 140 + 2*barprev + 5, 45, 2 * (barpercent - barprev), WHITE);
      } else {
        tft.fillRect(44, 140 + 2*barpercent + 5, 45, 2 * (barprev - barpercent), BLACK);
        tft.fillRect(44, 140 + 2*barpercent, 45, 2 * (barprev - barpercent), WHITE);
      }
    }
    disp_updated = false;
  }

  if (lock) {
    tft.setTextSize(3);
    if (warnings[0] != current_warnings[0]) {
      tft.setCursor(158, 205);
      if (warnings[0]) {
        tft.setTextColor(YELLOW, BLACK);
      } else {
        tft.setTextColor(DARKGRAY, BLACK);
      }
      current_warnings[0] = !current_warnings[0];
      tft.print(F("RELEASE"));
    }
    if (warnings[1] != current_warnings[1]) {
      tft.setCursor(158, 255);
      if (warnings[1]) {
        tft.setTextColor(YELLOW, BLACK);
      } else {
        tft.setTextColor(DARKGRAY, BLACK);
      }
      current_warnings[1] = !current_warnings[1];
      tft.print(F("SLOWER"));
    }
    if (warnings[2] != current_warnings[2]) {
      tft.setCursor(158, 305);
      if (warnings[2]) {
        tft.setTextColor(YELLOW, BLACK);
      } else {
        tft.setTextColor(DARKGRAY, BLACK);
      }
      current_warnings[2] = !current_warnings[2];
      tft.print(F("FASTER"));
    }
    if (warnings[3] != current_warnings[3]) {
      tft.setCursor(158, 355);
      if (warnings[3]) {
        tft.setTextColor(YELLOW, BLACK);
      } else {
        tft.setTextColor(DARKGRAY, BLACK);
      }
      current_warnings[3] = !current_warnings[3];
      tft.print(F("DEEPER"));
    }
    if (current_freq != cnt) {
      tft.fillRect(245, 115, 30, 40, BLACK); // for wiping out previous count data
      tft.setCursor(245, 115);
      tft.setTextSize(2);
      tft.setTextColor(WHITE, BLACK);
      tft.print(cnt);
      current_freq = cnt;
    }
  }

  if (imu.dataAvailable() == true) {
    if (count > 50 && started) {
      z = imu.getLinAccelZ();
      currentMillis = millis();
      last_few_pressures[0] = last_few_pressures[1];
      last_few_pressures[1] = last_few_pressures[2];
      last_few_pressures[2] = last_few_pressures[3];
      last_few_pressures[3] = last_few_pressures[4];
      last_few_pressures[4] = last_few_pressures[5];
      last_few_pressures[5] = last_few_pressures[6];
      last_few_pressures[6] = last_few_pressures[7];
      last_few_pressures[7] = last_few_pressures[8];
      last_few_pressures[8] = last_few_pressures[9];
      last_few_pressures[9] = pressure;
      if (abs(z) > 0.35) {
        velocity += (double)(interval) * (z + prev_z) / (2000);
        adj_acc_count = 0;
        disp_updated = true;
      } else {
        acc_count++;
        adj_acc_count++;
        inactivitycnt++;
      }
      displacement += (double)(interval) * (velocity + prev_vel) / (2000);
      disp_updated = true;
      p_pressure = pressure; 
      pressure = analogRead(pressure_pin);
      min_pressure = min(pressure, min_pressure);
      diff = currentMillis - previousMillis;

      if (displacement < deepest) {
        deepest = displacement;
        deep_time = millis();
      }

      if (displacement >= 0) {
        displacement = 0;
        disp_updated = true;
      }

      if (lock) {
        displacement = 0;
        velocity = 0;
        disp_updated = true;
        if (!speed_warning_updated && deep_time - prev_deep_time > 300) {
          cnt++;
          speed_warning_updated = true;
          bpm = (float) 60 * 1000 / (deep_time - prev_deep_time);
          if (prev_deep_time != 0) {
            lock_diff = (deep_time - prev_deep_time);
            if (n_bpm > 1) {
              if (bpm > 120) {
                warnings_count[2] = 0;
                warnings_count[1]++;
                total_warnings_count[1]++;
              } else if (bpm < 100) {
                warnings_count[1] = 0;
                warnings_count[2]++;
                total_warnings_count[2]++;
              } else {
                warnings_count[1] = 0;
                warnings_count[2] = 0;
              }
            }

            if (n_bpm == 0) {
              average_bpm = bpm;
            } else {
              average_bpm -= (average_bpm - bpm) / (n_bpm + 1);
            }
            n_bpm++;

            if (n_depth == 0) {
              average_depth = deepest;
            } else {
              average_depth -= (average_depth - deepest) / (n_depth + 1);
            }
            n_depth++;

            warnings[1] = (warnings_count[1] > 1) ? true : false;
            warnings[2] = (warnings_count[2] > 1) ? true : false;
            
            if (n_bpm > 1) {

              if (deepest > -0.04) {
                warnings_count[3]++;
                total_warnings_count[3]++;
              }
              else {
                warnings_count[3] = 0;
              }
              warnings[3] = (warnings_count[3] > 1) ? true : false;
            }
          }

          prev_deep_time = deep_time;
          deepest = 0;
        }

        release_updated = false;
      }

      if (pressure < 30 && abs(velocity) < 0.1 && z - prev_z > 0 && z <= 0 && velocity - prev_vel < 0) {
        lock = true;
        velocity = 0;
        pressure_reset_once = true;
      }

      if (pressure <= 10 && !pressure_reset_once && z - prev_z > 0) {
        lock = true;
        velocity = 0;
        pressure_reset_once = true;
      }

      if (pressure > 10 && pressure < 50 && displacement >= 0 && z < -0.1 && z - prev_z < 0 && pressure - p_pressure > 0) {
        lock = false;   
        lock_count = 0;
      }

      if (stdev() <= 4  && pressure < 20 && !pressure_reset_once) {
        lock = true;
        velocity = 0;
        displacement = 0;
        pressure_reset_once = true;
      }

      if (pressure < 30) {
        below_threshold_count++;
      }
      else {
        below_threshold_count = 0;
      }

      if (pressure > 40) {
        last_pump_time = millis();
        pressure_reset_once = false;
        speed_warning_updated = false;
        
        if (!release_updated) {
          if (min_pressure < 20) {
            warnings_count[0] = 0;
          }
          else {
            warnings_count[0]++;
            total_warnings_count[0]++;
          }
          warnings[0] = (warnings_count[0] > 1) ? true : false;
          release_updated = true;
          min_pressure = 1000;
        }
      }

      if (adj_acc_count > 20) {
        displacement = 0;
        adj_acc_count = 0;
        disp_updated = true;
      }

      if (acc_count > 20 && adj_acc_count > 5) {
        velocity = 0;
        prev_vel = 0;
        acc_count = 0;
      }

      if (serialOn) {

        Serial.print(10 * displacement, 3);
        Serial.print(F(","));
        Serial.print(velocity);
        Serial.print(F(","));
        if (abs(z) < 0.35)
          Serial.print(0.0, 3);
        else
          Serial.print(z / 10, 3);
        Serial.print(",");
        Serial.print(pressure);
        Serial.print(",");
        Serial.print(diff);
        Serial.print(",");
        Serial.print(average_bpm);
        Serial.print(",");
        Serial.print(average_depth);
        Serial.println();
      }
      prev_disp = displacement;
      previousMillis = currentMillis;
      prev_z = z;
      prev_vel = velocity;
    }
    count++;
  }
}
