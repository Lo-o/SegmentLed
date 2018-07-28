
#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h> // use either brightnessbus or neopixelbus.
#include <NeoPixelBus.h>

const uint16_t PixelCount = 56; // 7 segments, each 8 LEDs
const uint8_t PixelPin = 22;  // Used in this case

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);


// NeoPixel animation time management object
NeoPixelAnimator animations(PixelCount, NEO_MILLISECONDS);



void SetRandomSeed()
{
    uint32_t seed;

    // random works best with a seed that can use 31 bits
    // analogRead on a unconnected pin tends toward less than four bits
    seed = analogRead(0);
    delay(1);

    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }

    // Serial.println(seed);
    randomSeed(seed);
}



void setup()
{
    Serial.begin(115200);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

    SetRandomSeed();

    // Give all pixels in the string a random color.
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        //RgbColor color = RgbColor(random(255), random(255), random(255));
        RgbColor red = RgbColor(255,0,0); // added this, remove later + change in line below back to color        
        strip.SetPixelColor(pixel, red);
    }    
   
    Serial.println();
    Serial.println("Running...");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetupAnimationSet()
{
    // setup some animations
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        const uint8_t peak = 128;

        // pick a random duration of the animation for this pixel
        // since values are centiseconds, the range is 1 - 4 seconds
        uint16_t time = random(100, 400);

        // each animation starts with the color that was present
        RgbColor originalColor = strip.GetPixelColor(pixel);
        // and ends with a random color
        RgbColor targetColor = RgbColor(random(peak), random(peak), random(peak));
        // with the random ease function
        AnimEaseFunction easing;

        switch (random(3))
        {
        case 0:
            easing = NeoEase::CubicIn;
            Serial.println();
            Serial.println("case 0: CubicIn");
            break;
        case 1:
            easing = NeoEase::CubicOut;
            Serial.println();
            Serial.println("case 1: CubicOut");
            break;
        case 2:
            easing = NeoEase::QuadraticInOut;
            Serial.println();
            Serial.println("case 2: QaudraticInOut");
            break;
        }

#ifdef ARDUINO_ARCH_AVR
        // each animation starts with the color that was present
        animationState[pixel].StartingColor = originalColor;
        // and ends with a random color
        animationState[pixel].EndingColor = targetColor;
        // using the specific curve
        animationState[pixel].Easeing = easing;

        // now use the animation state we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animations.StartAnimation(pixel, time, AnimUpdate);
#else
        // we must supply a function that will define the animation, in this example
        // we are using "lambda expression" to define the function inline, which gives
        // us an easy way to "capture" the originalColor and targetColor for the call back.
        //
        // this function will get called back when ever the animation needs to change
        // the state of the pixel, it will provide a animation progress value
        // from 0.0 (start of animation) to 1.0 (end of animation)
        //
        // we use this progress value to define how we want to animate in this case
        // we call RgbColor::LinearBlend which will return a color blended between
        // the values given, by the amount passed, hich is also a float value from 0.0-1.0.
        // then we set the color.
        //
        // There is no need for the MyAnimationState struct as the compiler takes care
        // of those details for us
        AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
        {
            // progress will start at 0.0 and end at 1.0
            // we convert to the curve we want
            float progress = easing(param.progress);

            // use the curve value to apply to the animation
            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, targetColor, progress);
            strip.SetPixelColor(pixel, updatedColor);
        };

        // now use the animation properties we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animations.StartAnimation(pixel, time, animUpdate);
#endif
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void loop()
{

    // show the results
    strip.Show();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// BRIGHTNESS SKETCH
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#include <NeoPixelAnimator.h>
//#include <NeoPixelBrightnessBus.h> // use either brightnessbus or neopixelbus.
//#include <NeoPixelBus.h>
//
//const uint16_t PixelCount = 56; // 7 segments, each 8 LEDs
//const uint8_t PixelPin = 22;  // Used in this case
//
//#define colorSaturation 255 // saturation of color constants here 255 instead of 128?
//RgbColor red(colorSaturation, 0, 0);
//RgbColor green(0, colorSaturation, 0);
//RgbColor blue(0, 0, colorSaturation);
//RgbColor yellow(colorSaturation, colorSaturation, 0);
//
//// Address the 'neopixelbrightnessbus' instead of 'neopixelbus'.
//NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
//
//// you lose the original color the lower the dim value used
//// here due to quantization (?)
//const uint8_t c_MinBrightness = 8; 
//const uint8_t c_MaxBrightness = 255;
//
//int8_t direction; // placeholder, will hold the current direction of dimming -1 --> dim
//
//
//void setup()
//{
//    Serial.begin(115200);
//    while (!Serial); // wait for serial attach
//
//    Serial.println();
//    Serial.println("Initializing...");
//    Serial.flush();
//
//    // this resets all the neopixels to an off state
//    strip.Begin();
//    strip.Show();
//
//    direction = -1; // default to dim first
//    
//    Serial.println();
//    Serial.println("Running...");
//
//    // set our three original colors
//    // SetpixelColor --> strip.SetpixelColor(pixelcount, color); with color defined as RGB or HslColor.  
//    strip.SetPixelColor(0, red);
//    strip.SetPixelColor(1, green);
//    strip.SetPixelColor(2, blue);
//    strip.SetPixelColor(3, yellow);
//    strip.SetPixelColor(4, red);
//
//    for(int i = 24; i<=31; i++){
//      strip.SetPixelColor(i, red);
//    }
//    
//    strip.Show();
//}
//
//
//void loop()
//{
//    uint8_t brightness = strip.GetBrightness();
//    Serial.println(brightness);
//
//    // delay, again probably not the best way. This is the delay which determines in what time the brightness goes through a full cycle.
//    delay(100);
//
//    // swap diection of dim when limits are reached
//    //
//    if (direction < 0 && brightness <= c_MinBrightness)
//    {
//      direction = 1;
//    }
//    else if (direction > 0 && brightness >= c_MaxBrightness)
//    {
//      direction = -1;
//    }
//    // apply dimming
//    brightness += direction;  // brightness as a value only, initialized below
//    strip.SetBrightness(brightness);
//
//    // show the results
//    strip.Show();
//}
//





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEST SKETCH
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//// initialize the bus and define parameters. Standard method here. For different strip or ESP866 also different methods. 
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

//// Define color names as numeric values for GRB. Max number for each is 128.
//#define colorSaturation 128
//// e.g. red is 100% red, 0% green, 0% blue.
//RgbColor red(colorSaturation, 0, 0);
//RgbColor green(0, colorSaturation, 0);
//RgbColor blue(0, 0, colorSaturation);
//RgbColor white(colorSaturation);
//RgbColor black(0);
//
////HslColor: Hue, saturation, lightness. In this case translate RGBcolor to HSLcolor. 
//// ALSO: see template <typename T_NEOHUEBLEND> static HslColor LinearBlend(HslColor left, HslColor right, float progress) for blending between colors.
//HslColor hslRed(red);
//HslColor hslGreen(green);
//HslColor hslBlue(blue);
//HslColor hslWhite(white);
//HslColor hslBlack(black);
//
//void setup()
//{
//    Serial.begin(115200);
//    while (!Serial); // wait for serial attach
//
//    Serial.println();
//    Serial.println("Initializing...");
//    Serial.flush(); // Wait for the transmission of outgoing serial data to complete. 
//
//    // this resets all the neopixels to an off state
//    strip.Begin();
//    strip.Show();
//
//
//    Serial.println();
//    Serial.println("Running...");
//}
//
//
//void loop() {
//  delay(5000);
//
//  // Define each pixel color individually
//  strip.SetPixelColor(0, red);
//  strip.SetPixelColor(1, green);
//  strip.SetPixelColor(2, blue);
//  strip.SetPixelColor(3, white);
//  strip.Show();
//  
//  // Very easy, just delay here. 
//  delay(5000);
//
//  // Same, but with HslColor defined colors. Should be exactly the same colors. 
//  strip.SetPixelColor(0, hslRed);
//  strip.SetPixelColor(1, hslGreen);
//  strip.SetPixelColor(2, hslBlue);
//  strip.SetPixelColor(3, hslWhite);
//  strip.Show();
//
//  delay(5000);
//  // turn off the pixels
//  strip.SetPixelColor(0, hslBlack);
//  strip.SetPixelColor(1, hslBlack);
//  strip.SetPixelColor(2, hslBlack);
//  strip.SetPixelColor(3, hslBlack);
//  strip.Show();
//
//  
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
