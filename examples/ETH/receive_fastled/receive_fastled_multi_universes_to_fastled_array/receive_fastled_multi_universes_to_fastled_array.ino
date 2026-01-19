#include <FastLED.h>  // include FastLED *before* Artnet

#include <ArtnetETH.h> // used with ESP32 POE from Olimex

// Ethernet
const IPAddress ip(10, 10, 10, 2);
const IPAddress gateway(10, 10, 10, 10);
const IPAddress subnet(255, 255, 255, 0);

// start universe
uint8_t startuniverse = 0;

// initialize artnet
ArtnetETHReceiver artnet;

// set up FastLED for multiple pins with LED strips
// https://github.com/fastled/fastled/wiki/multiple-controller-examples#one-array-many-strips

// define LED pins
const uint8_t PIN_LED_DATA = 4;

#define COLOR GRB
#define LED_TYPE WS2812

#define NUM_LEDS_UNIVERSE 170

// number of LEDs in each universe
#define NUM_LEDS_u0 NUM_LEDS_UNIVERSE // L & A
#define NUM_LEDS_u1 155               // B und alle restlichen LEDs

// total number of LEDs is SUM of LEDs from each pin
#define NUM_TOTAL_LEDS NUM_LEDS_u0 + NUM_LEDS_u1

// create CRGB FastLED arrays
// CRGB leds_u0[NUM_LEDS_u0]; // L A
// CRGB leds_u1[NUM_LEDS_u1]; // B


CRGB leds_u_0_1[NUM_TOTAL_LEDS]; // B

void setup() {
    Serial.begin(115200);
    delay(2000);

    // create LED controller
    FastLED.addLeds<LED_TYPE, PIN_LED_DATA,  COLOR>(leds_u_0_1,NUM_TOTAL_LEDS);

    // FastLED.addLeds<LED_TYPE, PIN_LED_DATA,  COLOR>(leds_u_0_1,NUM_LEDS_u0, NUM_LEDS_u1);

    // start ethernet interface
    ETH.begin();
    ETH.config(ip, gateway, subnet);
    artnet.begin();

    // if Artnet packet comes to this universe, forward them to fastled directly
    // only for the first universe possible
    // artnet.forwardArtDmxDataToFastLED      (startuniverse,     leds_u_0_1, NUM_LEDS_u0); // can only digest up to 512 channels or 170 LEDs so NUM_LEDS_u0 can only be 170 at max
    // artnet.forwardArtDmxDataToFastLEDoffset(startuniverse,     leds_u_0_1, 0,           NUM_LEDS_u0);
    // artnet.forwardArtDmxDataToFastLEDoffset(startuniverse + 1, leds_u_0_1, NUM_LEDS_u0, NUM_LEDS_u1); // works only when number of LEDs fit in one universe i.e. NUM_TOTAL_LEDS-NUM_LEDS_u0 <= 170
    
    artnet.subscribeArtDmx(onArtDmx);


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

  Serial.print("ArtnetNode Started on");
  Serial.println(" 10.10.10.2");
  Serial.print(NUM_TOTAL_LEDS);
  Serial.println(" LEDs");
  Serial.print("Receiver for multiple Universes on µC pin ");
  Serial.println(PIN_LED_DATA);
  Serial.print("Receiving from ");
  // Serial.println(ArtNetRemoteInfo.IPAddress);
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



void onArtDmx(const uint8_t *data, uint16_t size, const ArtDmxMetadata& metadata, const ArtNetRemoteInfo& remote)
{
    Serial.print("received ArtNet data from ");
    Serial.print(remote.ip);
    Serial.print(":");
    Serial.print(remote.port);
    Serial.print(", universe = ");
    Serial.print(metadata.universe);
    Serial.print(", sequence = ");
    Serial.print(metadata.sequence);
    Serial.print(", size = ");
    Serial.print(size);
    Serial.println(")");
};