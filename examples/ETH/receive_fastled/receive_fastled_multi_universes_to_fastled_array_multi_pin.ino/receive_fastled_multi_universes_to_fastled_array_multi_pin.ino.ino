#include <FastLED.h>  // include FastLED *before* Artnet

#include <ArtnetETH.h> // used with ESP32 POE from Olimex
// #include <ArtnetNativeEther.h>  // only for Teensy 4.1

// Ethernet
const IPAddress ip(10, 10, 10, 100);
const IPAddress gateway(10, 10, 10, 10);
const IPAddress subnet(255, 255, 255, 0);

// total number of LEDs is SUM of LEDs from each pin
#define NUM_LEDS_VOGEL  410
#define NUM_LEDS_JAZZ   183
#define NUM_LEDS_MONTEZ 145

#define NUM_TOTAL_LEDS NUM_LEDS_VOGEL + NUM_LEDS_JAZZ + NUM_LEDS_MONTEZ

// start universe
uint8_t startuniverse = 0;

// initialize artnet
ArtnetETHReceiver artnet;

// set up FastLED for multiple pins with LED strips
// https://github.com/fastled/fastled/wiki/multiple-controller-examples#one-array-many-strips

// define LED pins
const uint8_t PIN_LED_DATA_1 = 16;
const uint8_t PIN_LED_DATA_2 = 5;
const uint8_t PIN_LED_DATA_3 = 4;

#define COLOR_ORDER GRB
#define LED_TYPE WS2812

#define NUM_LEDS_UNIVERSE 170



// number of LEDs in each universe of pin X
#define NUM_LEDS_PIN1_U0 NUM_LEDS_UNIVERSE
#define NUM_LEDS_PIN1_U1 NUM_LEDS_UNIVERSE
#define NUM_LEDS_PIN1_U2 NUM_LEDS_VOGEL - 2 * NUM_LEDS_UNIVERSE

#define NUM_LEDS_PIN2_U3 NUM_LEDS_UNIVERSE
#define NUM_LEDS_PIN2_U4 NUM_LEDS_JAZZ - NUM_LEDS_UNIVERSE

#define NUM_LEDS_PIN3_U5 NUM_LEDS_MONTEZ


// fastLED start indices for each universe
const uint16_t start0 = 0;
const uint16_t start1 = NUM_LEDS_PIN1_U0;
const uint16_t start2 = start1 + NUM_LEDS_PIN1_U1;
const uint16_t start3 = start2 + NUM_LEDS_PIN1_U2;
const uint16_t start4 = start3 + NUM_LEDS_PIN2_U3;
const uint16_t start5 = start4 + NUM_LEDS_PIN2_U4;



// create LED array for FastLED
CRGB leds[NUM_TOTAL_LEDS]; // B

void setup() {
    Serial.begin(115200);
    delay(2000);

    // create LED controller
    FastLED.addLeds<LED_TYPE, PIN_LED_DATA_1,  COLOR_ORDER>(leds, NUM_LEDS_VOGEL);
    FastLED.addLeds<LED_TYPE, PIN_LED_DATA_2,  COLOR_ORDER>(leds, NUM_LEDS_JAZZ);
    FastLED.addLeds<LED_TYPE, PIN_LED_DATA_3,  COLOR_ORDER>(leds, NUM_LEDS_MONTEZ);

    // start ethernet interface
    ETH.begin();
    ETH.config(ip, gateway, subnet);
    artnet.begin();

    // stats call to show which universes are beeing received
    // artnet.subscribeArtDmx(onArtDmx);

    // if Artnet packet comes to this universe, forward them to fastled directly
    // only for the first universe possible
    artnet.forwardArtDmxDataToFastLED      (startuniverse,     leds, NUM_LEDS_PIN1_U0); // can only digest up to 512 channels or 170 LEDs so NUM_LEDS_u0 can only be 170 at max
    // artnet.forwardArtDmxDataToFastLEDoffset(startuniverse,     leds_u_0_1, 0,           NUM_LEDS_u0);
    artnet.forwardArtDmxDataToFastLEDoffset(startuniverse + 1, leds, start1, NUM_LEDS_PIN1_U1); // works only when number of LEDs fit in one universe i.e. NUM_LEDS_PIN <= 170
    artnet.forwardArtDmxDataToFastLEDoffset(startuniverse + 2, leds, start2, NUM_LEDS_PIN1_U2); // works only when number of LEDs fit in one universe i.e. NUM_LEDS_PIN <= 170
    artnet.forwardArtDmxDataToFastLEDoffset(startuniverse + 3, leds, start3, NUM_LEDS_PIN2_U3); // works only when number of LEDs fit in one universe i.e. NUM_LEDS_PIN <= 170
    artnet.forwardArtDmxDataToFastLEDoffset(startuniverse + 4, leds, start4, NUM_LEDS_PIN2_U4); // works only when number of LEDs fit in one universe i.e. NUM_LEDS_PIN <= 170
    artnet.forwardArtDmxDataToFastLEDoffset(startuniverse + 5, leds, start5, NUM_LEDS_PIN3_U5); // works only when number of LEDs fit in one universe i.e. NUM_LEDS_PIN <= 170


    // this can be achieved manually as follows
    // artnet.subscribeArtDmxUniverse(startuniverse + 1, [](const uint8_t *data, uint16_t size, const ArtDmxMetadata& metadata, const ArtNetRemoteInfo& remote)
    // {
    //   Serial.print("received ArtNet data from ");
    //     // artnet data size per packet is 512 max
    //     // so there is max 170 pixel per packet (per universe)
    //     for (size_t pixel = NUM_LEDS_u0; pixel < NUM_TOTAL_LEDS; ++pixel)
    //     {
    //         size_t idx = (pixel-NUM_LEDS_u0) * 3;
    //         leds_u_0_1[pixel].r = data[idx + 0];
    //         leds_u_0_1[pixel].g = data[idx + 1];
    //         leds_u_0_1[pixel].b = data[idx + 2];
    //     }
    // });

  Serial.print("ArtnetNode Started on ");
  Serial.println(ETH.localIP());
  Serial.print(NUM_TOTAL_LEDS);
  Serial.println(" LEDs");
  Serial.print("Receiver for multiple Universes on multiple µC pins ");
  Serial.println(PIN_LED_DATA_1);
  Serial.print(", ");
  Serial.println(PIN_LED_DATA_2);
  Serial.print(", ");
  Serial.println(PIN_LED_DATA_3);
}

void loop() {
    artnet.parse();  // check if artnet packet has come and execute callback
    FastLED.show();  // call FastLED to show new data to the LEDs

  // for(int i = 0; i < NUM_TOTAL_LEDS; i++) {
  //   leds_u_0_1[i] = CRGB::Red;    // set our current dot to red
  //   FastLED.show();
  //   leds_u_0_1[i] = CRGB::Black;  // set our current dot to black before we continue
  // }
}


// function to show stats on Serial
void onArtDmx(const uint8_t *data, uint16_t size, const ArtDmxMetadata& metadata, const ArtNetRemoteInfo& remote)
{
    Serial.print("received ArtNet universes: ");
    Serial.println(metadata.universe);
    // Serial.print("received ArtNet data from ");
    // Serial.print(remote.ip);
    // Serial.print(":");
    // Serial.print(remote.port);
    // Serial.print(", universe = ");
    // Serial.print(metadata.universe);
    // Serial.print(", size = ");
    // Serial.print(size);
    // Serial.println(")");
};

