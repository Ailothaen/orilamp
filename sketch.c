#include <RGBConverter.h>
#include <EEPROM.h>
#include <math.h>

RGBConverter RGBlib;

// Pins
int inH = A0; // H = red button, controlling Hue
int inL = A1; // L = white button, controlling Light
int inE = A2; // E = green button, controlling Effect
int outR = 3;
int outG = 5;
int outB = 6;

// Admissible values for Hue, Saturation, Light and Chance/Speed
double Hscope[] = {0.00, 0.01, 0.02, 0.03, 0.04, 0.06, 0.08, 0.10, 0.13, 0.17, 0.22, 0.28, 0.35, 0.4, 0.45, 0.5, 0.53, 0.56, 0.6, 0.64, 0.68, 0.70, 0.72, 0.74, 0.76, 0.8, 0.84, 0.88, 0.91, 0.93, 0.95, 0.97, 0.99};
double Sscope[] = {0.00, 0.25, 0.50, 0.75, 1};
double Lscope[] = {0.16, 0.33, 0.50, 0.75, 1};
int Cscope[] = {1000, 2000, 3500, 5000, 7500, 10000};

// Sizes of arrays above
int Hscope_len = sizeof(Hscope)/sizeof(Hscope[0]);
int Sscope_len = sizeof(Sscope)/sizeof(Sscope[0]);
int Lscope_len = sizeof(Lscope)/sizeof(Lscope[0]);
int Escope_len = 6;
int Cscope_len = sizeof(Lscope)/sizeof(Lscope[0]);

// EEPROM addresses, where the data is saved
int addrH = 0;
int addrS = 1;
int addrL = 2;
int addrE = 3;
int addrC = 4;

// Config
bool cfSerialEnabled = false;

// Runtime variables
int nowH; // current value of H, S, L and E
int nowS;
int nowL;
int nowE;
int nowC; // speed/chance of effects
byte RGB[3];
char message[64]; // serial output for sprintf
unsigned long prevMillis;
int inHstatenow; // input value of buttons now
int inHstateprev; // input value of buttons at previous loop
int inLstatenow;
int inLstateprev;
int inEstatenow;
int inEstateprev;
bool cancelHpress = false; // If there is a composite command (2 buttons at the same time),
bool cancelLpress = false; // we must register that the first button was pressed "because" of the second
bool cancelEpress = false; // to not trigger the action of the first button on release


void color(int h, int s, int l)
{
    RGBlib.hslToRgb(Hscope[h], Sscope[s], Lscope[l], RGB);
    analogWrite(outR, RGB[0]);
    analogWrite(outG, RGB[1]);
    analogWrite(outB, RGB[2]);
}


// "I told them to build a statue of me"...
void color_raw(double h, double s, double l)
{
    RGBlib.hslToRgb(h, s, l, RGB);
    analogWrite(outR, RGB[0]);
    analogWrite(outG, RGB[1]);
    analogWrite(outB, RGB[2]);
}


double clock(double min, double max, unsigned long ms, unsigned long ms_for_cycle)
{
    double progression = (ms % ms_for_cycle) / (double)ms_for_cycle;
    progression = fabs((progression*2)-1);
    progression = progression*(max-min)+min;
    return(progression);
}


double clock_blink(unsigned long light, unsigned long dark, unsigned long ms)
{
    double progression = (ms % (light+dark)) / (double)(light+dark);
    double progression_light = light/(double)(light+dark);
    return(progression < progression_light);
}


// Yes, 2 echo functions is a bit dirty...
void echo(char const *message2)
{
    if(cfSerialEnabled)
    {
        Serial.println(message2);
    }
}


// this one to be used with sprintf
void echof(int no_one_cares)
{
    if(cfSerialEnabled)
    {
        Serial.println(message);
    }
}


void setup()
{
    // Initializing pins
    pinMode(inH, INPUT);
    pinMode(inL, INPUT);
    pinMode(inE, INPUT);
    pinMode(outR, OUTPUT);
    pinMode(outG, OUTPUT);
    pinMode(outB, OUTPUT);

    if(analogRead(inE) > 800)
    {
        // If E is pressed at startup:
        // Enable serial output
        // Note that E will have to be held when the connection is opened, NOT when the board is turned on
        // (cf. https://forum.arduino.cc/t/does-arduino-reset-when-first-connected-to-usb/544278)
        cancelEpress = true;
        cfSerialEnabled = true;
        Serial.begin(9600);
        echo("Serial connected");
    }

    if(analogRead(inL) > 800)
    {
        // If L is pressed at startup:
        // Reset values
        cancelLpress = true;
        EEPROM.update(addrH, 14);
        EEPROM.update(addrS, 4);
        EEPROM.update(addrL, 2);
        EEPROM.update(addrE, 0);
        EEPROM.update(addrC, 3);
    }

    // Reading now values from EEPROM
    nowH = EEPROM.read(addrH);
    nowS = EEPROM.read(addrS);
    nowL = EEPROM.read(addrL);
    nowE = EEPROM.read(addrE);
    nowC = EEPROM.read(addrC);

    // Checking if they are correct...
    if(nowH >= Hscope_len)
    {
        echo("[ERROR] Read wrong nowH value from EEPROM");
        nowH = 14;
    }

    if(nowS >= Sscope_len)
    {
        echo("[ERROR] Read wrong nowS value from EEPROM");
        nowS = 4;
    }

    if(nowL >= Lscope_len)
    {
        echo("[ERROR] Read wrong nowL value from EEPROM");
        nowL = 2;
    }

    if(nowE >= Escope_len)
    {
        echo("[ERROR] Read wrong nowE value from EEPROM");
        nowE = 0;
    }

    if(nowC >= Cscope_len)
    {
        echo("[ERROR] Read wrong nowC value from EEPROM");
        nowC = 3;
    }

    // Initializing LED
    color(nowH, nowS, nowL);

    // Initiating PRNG
    randomSeed(analogRead(A5));
}


void loop()
{
    // Constantly watching for button press&releases.
    inHstatenow = analogRead(inH);
    inLstatenow = analogRead(inL);
    inEstatenow = analogRead(inE);

    // Button H held + button L released
    // Change Saturation
    if(inHstatenow > 800 && inLstateprev > 800 && inLstatenow < 800)
    {
        echo("[PRESS] H hold + L release");
        cancelHpress = true;
        nowS++;
        if(nowS >= Sscope_len)
        {
            nowS = 0;
        }
        echof(sprintf(message, "[VALUE] nowS=%d", nowS));
        color(nowH, nowS, nowL);
        EEPROM.update(addrS, nowS);
    }

    // Button E held + button L released
    // Change Speed/Chance of effects
    else if(inEstatenow > 800 && inLstateprev > 800 && inLstatenow < 800)
    {
        echo("[PRESS] E hold + L release");
        cancelEpress = true;
        nowC++;
        if(nowC >= Cscope_len)
        {
            nowC = 0;
        }
        echof(sprintf(message, "[VALUE] nowC=%d", nowC));
        EEPROM.update(addrC, nowC);
    }

    // Button H released
    // Change Hue
    else if(inHstatenow < 800 && inHstateprev > 800)
    {
        if(cancelHpress)
        {
            cancelHpress = false;
        }
        else
        {
            echo("[PRESS] H release");
            nowH++;
            if(nowH >= Hscope_len)
            {
                nowH = 0;
            }
            echof(sprintf(message, "[VALUE] nowH=%d", nowH));
            color(nowH, nowS, nowL);
            EEPROM.update(addrH, nowH);
        }
    }

    // Button L released
    // Change Lightness
    else if(inLstatenow < 800 && inLstateprev > 800)
    {
        if(cancelLpress)
        {
            cancelLpress = false;
        }
        else
        {
            echo("[PRESS] L release");
            nowL++;
            if(nowL >= Lscope_len)
            {
                nowL = 0;
            }
            echof(sprintf(message, "[VALUE] nowL=%d", nowL));
            color(nowH, nowS, nowL);
            EEPROM.update(addrL, nowL);
        }
    }

    // Button E released
    // Change Effect
    else if(inEstatenow < 800 && inEstateprev > 800)
    {
        if(cancelEpress)
        {
            cancelEpress = false;
        }
        else
        {
            echo("[PRESS] E release");
            nowE++;
            if(nowE >= Escope_len)
            {
                nowE = 0;
            }
            echof(sprintf(message, "[VALUE] nowE=%d", nowE));
            EEPROM.update(addrE, nowE);
        }
    }

    // Managing effect if there is one active
    // 0 None
    // 1 Glow
    // 2 Glow to black
    // 3 Blink
    // 4 Blink to black
    // 5 Still but randomly changes color
    if(nowE == 1)
    {
        // 1 Glow: change L between 0.15 and 0.45
        double alteredL = clock(0.15, 0.45, millis(), Cscope[nowC]);
        double h = Hscope[nowH];
        double s = Sscope[nowS];
        color_raw(h, s, alteredL);
    }
    else if(nowE == 2)
    {
        // 2 Glow to black: change L between 0 and 0.45
        double alteredL = clock(0, 0.45, millis(), Cscope[nowC]);
        double h = Hscope[nowH];
        double s = Sscope[nowS];
        color_raw(h, s, alteredL);
    }
    else if(nowE == 3)
    {
        // 3 Blink
        bool is_lit = clock_blink(Cscope[nowC], 500, millis());
        double h = Hscope[nowH];
        double s = Sscope[nowS];
        double l = Lscope[nowL];
        if(is_lit)
        {
            color_raw(h, s, l);
        }
        else
        {
            color_raw(h, s, l/4);
        }
    }
    else if(nowE == 4)
    {
        // 4 Blink to black
        bool is_lit = clock_blink(Cscope[nowC], 500, millis());
        double h = Hscope[nowH];
        double s = Sscope[nowS];
        double l = Lscope[nowL];
        if(is_lit)
        {
            color_raw(h, s, l);
        }
        else
        {
            color_raw(h, s, 0);
        }
    }
    else if(nowE == 5)
    {
        // 5 Randomly changes color sometimes
        // Rolls every second
        if((millis()-prevMillis) > 1000)
        {
            echo("[INFO] Rolling...");
            long roll = random(Cscope[nowC]*0.03); // so lowest C is 1/30 chance

            if(roll == 11) // like March 11, of course :)
            {
                echo("[INFO] Roll successful!");
                long randomH = random(Hscope_len);
                nowH = randomH;
                echof(sprintf(message, "[VALUE] nowH=%d", nowH));
                color(nowH, nowS, nowL);
            }

            prevMillis = millis();
        }
    }

    inHstateprev = inHstatenow;
    inLstateprev = inLstatenow;
    inEstateprev = inEstatenow;
}
