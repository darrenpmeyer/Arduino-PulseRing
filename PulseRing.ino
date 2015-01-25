#include <FastLED.h>

/** 
 NOTE: enabling debug dramatically slows the animation
 You should really only do this if you _need_ to see how
 the values progress with each loop() "tick" in order to
 diagnose logic problems. Don't use it to debug timing!
**/
// #define DEBUG 
#undef DEBUG

#define LEDS 24
#define LED_PIN 2
#define LED_DRIVER NEOPIXEL

CHSV led[LEDS];
CRGB pixel[LEDS];

#define MAX_BRIGHT 192 // default for setBrightness as well as the max "value" for CSV
#define MIN_BRIGHT 20  // min "value" for CSV
#define MAX_HUE 255    // max "Hue" for CSV
#define MIN_HUE 1      // min "Hue" for CSV
#define MAX_SWEEP 24   // number of LEDs to be lit at once; mem use increases with value!

short val[MAX_SWEEP];
short hue[MAX_SWEEP];
bool val_dir[MAX_SWEEP];
bool hue_dir[MAX_SWEEP];

#define DOWN true
#define UP false

void setup()
{
	FastLED.addLeds<NEOPIXEL, LED_PIN>(pixel, LEDS);
	FastLED.setBrightness(MAX_BRIGHT);
	fill_solid(pixel, LEDS, 0);
	FastLED.show();
	delay(150);

	byte valsep = (MAX_BRIGHT - MIN_BRIGHT) / MAX_SWEEP;
	byte valhue = (MAX_HUE - MIN_HUE) / MAX_SWEEP;
	for (byte i = 0; i < MAX_SWEEP; i++) {
		val_dir[i] = UP;
		hue_dir[i] = UP;
		val[i] = i+valsep;   //initial "brightness" sep
		hue[i] = i+valhue;  //ititial hue sep
		led[i] = CHSV(hue[i],240,val[i]);
	}

	#ifdef DEBUG
	Serial.begin(57600);
	#endif
}

#define REFRESH_DELAY 42 // 42 = ceil(1000/24) = 24fps
#define VALSEP 14           // larger = faster brightness change
#define HUESEP 7         // larger = faster hue change

void loop()
{
	static byte master_offset;
	for (byte i = 0; i < LEDS; i++) {
		short offset = master_offset + i;
		if (offset >= LEDS) offset = offset-LEDS;
		if (offset < 0) offset = 0; // should never happen, freak!

		#ifdef DEBUG
		// debugging info to Serial port
		Serial.print(i);
		Serial.print(":");
		Serial.print(master_offset);
		Serial.print(":");
		Serial.println(offset);
		#endif

		if (i >= MAX_SWEEP) {
			pixel[offset] = 0;
			continue; 
		}

		if (val_dir[i] == UP) {
			// this pixel is incrementing value
			#ifdef DEBUG
			Serial.print(String("  Value UP: ") + val[i] + " => ");
			#endif

			val[i]+=VALSEP;
			if (val[i] >= MAX_BRIGHT) {
				val_dir[i] = DOWN;
				val[i] = MAX_BRIGHT;
			}
		}
		else {
			// this pixel is decrementing value
			#ifdef DEBUG
			Serial.print(String("  Value DN: ") + val[i] + " => ");
			#endif

			val[i]-=VALSEP;
			if (val[i] <= MIN_BRIGHT) {
				val_dir[i] = UP;
				val[i] = MIN_BRIGHT;
			}
		}
		#ifdef DEBUG
		Serial.println(val[i]);
		#endif

		if ( ! hue_dir[i] ) {
			// this pixel is incrementing hue
			#ifdef DEBUG
			Serial.print(String("  Hue   UP: ") + hue[i] + " => ");
			#endif
			hue[i]+=HUESEP;
			if (hue[i] >= MAX_HUE) {
				hue_dir[i] = DOWN;
				hue[i] = MAX_HUE;
			}
		}
		else {
			// this pixel is decrementing hue
			#ifdef DEBUG
			Serial.print(String("  Hue   DN: ") + hue[i] + " => ");
			#endif

			hue[i]-=HUESEP;
			if (hue[i] <= MIN_HUE) {
				hue_dir[i] = UP;
				hue[i] = MIN_HUE;
			}
		}
		#ifdef DEBUG
		Serial.println(hue[i]);
		#endif

		led[i].value = val[i];
		led[i].hue = hue[i];
		pixel[offset] = led[i];
	}

	if (master_offset < LEDS-1) master_offset++;
	else master_offset = 0;

	FastLED.show();
	FastLED.delay(REFRESH_DELAY);
}