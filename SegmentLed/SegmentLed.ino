//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Easing function / red to blue

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint16_t PixelCount = 56; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 22;  // make sure to set this to the correct pin, ignored for Esp8266

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// Start NeoPixelBus


// NeoPixel animation time management object
NeoPixelAnimator animations(PixelCount, NEO_CENTISECONDS);

// create with enough animations to have one per pixel, depending on the animation
// effect, you may need more or less.
//
// since the normal animation time range is only about 65 seconds, by passing timescale value
// to the NeoPixelAnimator constructor we can increase the time range, but we also increase
// the time between the animation updates.   
// NEO_CENTISECONDS will update the animations every 100th of a second rather than the default
// of a 1000th of a second, but the time range will now extend from about 65 seconds to about
// 10.9 minutes.  But you must remember that the values passed to StartAnimations are now 
// in centiseconds.
//
// Possible values from 1 to 32768, and there some helpful constants defined as...
// NEO_MILLISECONDS        1    // ~65 seconds max duration, ms updates
// NEO_CENTISECONDS       10    // ~10.9 minutes max duration, centisecond updates
// NEO_DECISECONDS       100    // ~1.8 hours max duration, decisecond updates
// NEO_SECONDS          1000    // ~18.2 hours max duration, second updates
// NEO_DECASECONDS     10000    // ~7.5 days, 10 second updates
//


struct MyAnimationState
{
    RgbColor StartingColor = RgbColor(255,0,0);  // the color the animation starts at (RGBColor is called, so red-green-blue 0-255)
    RgbColor EndingColor = RgbColor(0,255,255); // the color the animation will end at
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};

MyAnimationState animationState[PixelCount];
// one entry per pixel to match the animation timing manager

void AnimUpdate(const AnimationParam& param)
{
    // first apply an easing (curve) to the animation
    // this simulates acceleration to the effect
    float progress = NeoEase::ExponentialInOut(param.progress);

    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        progress);
    // apply the color to the strip
    strip.SetPixelColor(param.index, updatedColor);
}


void SetRandomSeed()
{
    // Basically make a proper random. Called in setup
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

    strip.Begin();
    strip.Show();

    SetRandomSeed();  // get proper random numbers when calling random()

    // just pick some colors
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        RgbColor color = RgbColor(random(255), random(255), random(255));
        strip.SetPixelColor(pixel, color);
    }

    Serial.println();
    Serial.println("Running...");
}


void SetupAnimationSet(uint8_t FunctionType)
{
    const int BlockSet1[] = {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23, 32, 33, 34, 35, 36, 37, 38, 39, 48, 49, 50, 51, 52, 53, 54, 55};
    const int BlockSet2[] = {8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31, 40, 41, 42, 43, 44, 45, 46, 47};
    
    const uint8_t peak = 255; // Max brightness value for a pixel

    if (FunctionType == 1)
    {
        // StartingColor by block
        RgbColor startingColor1 = HslColor(random(170, 230) / 360.0f, 0.1f, 0.5f);  // 1st set of blocks
        RgbColor startingColor2 = HslColor(random(170, 230) / 360.0f, 1.0f, 0.5f);  // 2nd set of blocks
        
        // and ends with a random color
        RgbColor targetColor1 = HslColor(random(170, 190) / 360.0f, 1.0f, 0.5f);
        RgbColor targetColor2 = HslColor(random(170, 230) / 360.0f, 1.0f, 0.5f);
        
        // with the random ease function
        AnimEaseFunction easing;
        easing = NeoEase::QuadraticInOut;

        // Block 1 animation setup 
        for (uint16_t i = 0; i < (sizeof(BlockSet1)/sizeof(int)); i++)
        { 
            uint16_t pixel = BlockSet1[i];
            RgbColor originalColor = strip.GetPixelColor(pixel);
            
            uint16_t time = random(700, 800);  // Random duration of animation. Value in centiseconds. 100 = 1sec.
    
            animationState[pixel].StartingColor = originalColor;  
            animationState[pixel].EndingColor = targetColor1;
            animationState[pixel].Easeing = easing;  // From startingColor to EndingColor using this specific curve
    
            // now use the animation state we just calculated and start the animation
            // which will continue to run and call the update function until it completes
            animations.StartAnimation(pixel, time, AnimUpdate);
        }
    
        // Block 2 animation setup
        for (uint16_t i = 0; i < (sizeof(BlockSet2)/sizeof(int)); i++)
        { 
            uint16_t pixel = BlockSet2[i];
            RgbColor originalColor = strip.GetPixelColor(pixel);
            
            uint16_t time = random(700, 800);  // Random duration of animation. Value in centiseconds. 100 = 1sec.
    
            animationState[pixel].StartingColor = originalColor;  
            animationState[pixel].EndingColor = targetColor2;
            animationState[pixel].Easeing = easing;  // From startingColor to EndingColor using this specific curve
    
            // now use the animation state we just calculated and start the animation
            // which will continue to run and call the update function until it completes
            animations.StartAnimation(pixel, time, AnimUpdate);
        }
    }

    if (FunctionType == 2)
    {
      for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
      {
        RgbColor originalColor = strip.GetPixelColor(pixel);
        RgbColor targetColor = HslColor(random(100, 300) / 360.0f, 1.0f, 0.5f);

        AnimEaseFunction easing;
        easing = NeoEase::QuadraticInOut;

        uint16_t time = random(700, 800);
        animationState[pixel].StartingColor = originalColor;
        animationState[pixel].EndingColor = targetColor;
        animationState[pixel].Easeing = easing;

        animations.StartAnimation(pixel, time, AnimUpdate);
      }
    }

    
}

void loop()
{
    if (animations.IsAnimating())
    {
        // the normal loop just needs these two to run the active animations
        animations.UpdateAnimations();
        strip.Show();
    }
    else
    {
        Serial.println();
        Serial.println("Setup Next Set...");

        uint16_t set = random(1, 3); // Randomly choose either one of the 3 functions
        SetupAnimationSet(set);   
        Serial.println(set);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Easing function / red to blue
//
//#include <NeoPixelBus.h>
//#include <NeoPixelAnimator.h>
//
//const uint16_t PixelCount = 56; // make sure to set this to the number of pixels in your strip
//const uint8_t PixelPin = 22;  // make sure to set this to the correct pin, ignored for Esp8266
//
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
//// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
//// There are other Esp8266 alternative methods that provide more pin options, but also have
//// other side effects.
////NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
////
//// NeoEsp8266Uart800KbpsMethod uses GPI02 instead
//
//
//// NeoPixel animation time management object
//NeoPixelAnimator animations(PixelCount, NEO_CENTISECONDS);
//
//// create with enough animations to have one per pixel, depending on the animation
//// effect, you may need more or less.
////
//// since the normal animation time range is only about 65 seconds, by passing timescale value
//// to the NeoPixelAnimator constructor we can increase the time range, but we also increase
//// the time between the animation updates.   
//// NEO_CENTISECONDS will update the animations every 100th of a second rather than the default
//// of a 1000th of a second, but the time range will now extend from about 65 seconds to about
//// 10.9 minutes.  But you must remember that the values passed to StartAnimations are now 
//// in centiseconds.
////
//// Possible values from 1 to 32768, and there some helpful constants defined as...
//// NEO_MILLISECONDS        1    // ~65 seconds max duration, ms updates
//// NEO_CENTISECONDS       10    // ~10.9 minutes max duration, centisecond updates
//// NEO_DECISECONDS       100    // ~1.8 hours max duration, decisecond updates
//// NEO_SECONDS          1000    // ~18.2 hours max duration, second updates
//// NEO_DECASECONDS     10000    // ~7.5 days, 10 second updates
////
//
//
//struct MyAnimationState
//{
//    RgbColor StartingColor = RgbColor(255,0,0);  // the color the animation starts at
//    RgbColor EndingColor = RgbColor(0,255,255); // the color the animation will end at
//    AnimEaseFunction Easeing; // the acceleration curve it will use 
//};
//
//MyAnimationState animationState[PixelCount];
//// one entry per pixel to match the animation timing manager
//
//void AnimUpdate(const AnimationParam& param)
//{
//    // first apply an easing (curve) to the animation
//    // this simulates acceleration to the effect
//    float progress = NeoEase::ExponentialInOut(param.progress);
//
//    // this gets called for each animation on every time step
//    // progress will start at 0.0 and end at 1.0
//    // we use the blend function on the RgbColor to mix
//    // color based on the progress given to us in the animation
//    RgbColor updatedColor = RgbColor::LinearBlend(
//        animationState[param.index].StartingColor,
//        animationState[param.index].EndingColor,
//        progress);
//    // apply the color to the strip
//    strip.SetPixelColor(param.index, updatedColor);
//}
//
//
//void SetRandomSeed()
//{
//    uint32_t seed;
//
//    // random works best with a seed that can use 31 bits
//    // analogRead on a unconnected pin tends toward less than four bits
//    seed = analogRead(0);
//    delay(1);
//
//    for (int shifts = 3; shifts < 31; shifts += 3)
//    {
//        seed ^= analogRead(0) << shifts;
//        delay(1);
//    }
//
//    // Serial.println(seed);
//    randomSeed(seed);
//}
//
//void setup()
//{
//    Serial.begin(115200);
//    while (!Serial); // wait for serial attach
//
//    strip.Begin();
//    strip.Show();
//
//    SetRandomSeed();
//
//    // just pick some colors
//    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
//    {
//        RgbColor color = RgbColor(random(255), random(255), random(255));
//        strip.SetPixelColor(pixel, color);
//    }
//
//    Serial.println();
//    Serial.println("Running...");
//}
//
//
//void SetupAnimationSet()
//{
//    // setup some animations
//    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
//    {
//        const uint8_t peak = 128;
//
//        // pick a random duration of the animation for this pixel
//        // since values are centiseconds, the range is 1 - 4 seconds
//        uint16_t time = random(100, 400);
//
//        // each animation starts with the color that was present
//        RgbColor originalColor = RgbColor(255,0,0);
//        // and ends with a random color
//        RgbColor targetColor = RgbColor(0,255,255);
//        // with the random ease function
//        AnimEaseFunction easing;
//        easing = NeoEase::QuadraticInOut;
//
//
//
//
//        // each animation starts with the color that was present
//        animationState[pixel].StartingColor = originalColor;
//        // and ends with a random color
//        animationState[pixel].EndingColor = targetColor;
//        // using the specific curve
//        animationState[pixel].Easeing = easing;
//
//        // now use the animation state we just calculated and start the animation
//        // which will continue to run and call the update function until it completes
//        animations.StartAnimation(pixel, time, AnimUpdate);
//
//    }
//}
//
//void loop()
//{
//    if (animations.IsAnimating())
//    {
//        // the normal loop just needs these two to run the active animations
//        animations.UpdateAnimations();
//        strip.Show();
//    }
//    else
//    {
//        Serial.println();
//        Serial.println("Setup Next Set...");
//        // example function that sets up some animations
//        SetupAnimationSet();
//    }
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







//
//#include <NeoPixelAnimator.h>
//#include <NeoPixelBrightnessBus.h> // use either brightnessbus or neopixelbus.
//#include <NeoPixelBus.h>
//
//const uint16_t PixelCount = 56; // 7 segments, each 8 LEDs
//const uint8_t PixelPin = 22;  // Used in this case
//
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
//
//
//// NeoPixel animation time management object
//NeoPixelAnimator animations(PixelCount, NEO_MILLISECONDS);
//
//
//
//void SetRandomSeed()
//{
//    uint32_t seed;
//
//    // random works best with a seed that can use 31 bits
//    // analogRead on a unconnected pin tends toward less than four bits
//    seed = analogRead(0);
//    delay(1);
//
//    for (int shifts = 3; shifts < 31; shifts += 3)
//    {
//        seed ^= analogRead(0) << shifts;
//        delay(1);
//    }
//
//    // Serial.println(seed);
//    randomSeed(seed);
//}
//
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
//    SetRandomSeed();
//
//    // Give all pixels in the string a random color.
//    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
//    {
//        //RgbColor color = RgbColor(random(255), random(255), random(255));
//        RgbColor red = RgbColor(255,0,0); // added this, remove later + change in line below back to color        
//        strip.SetPixelColor(pixel, red);
//    }    
//   
//    Serial.println();
//    Serial.println("Running...");
//}
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void SetupAnimationSet()
//{
//    // setup some animations
//    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
//    {
//        const uint8_t peak = 128;
//
//        // pick a random duration of the animation for this pixel
//        // since values are centiseconds, the range is 1 - 4 seconds
//        uint16_t time = random(100, 400);
//
//        // each animation starts with the color that was present
//        RgbColor originalColor = strip.GetPixelColor(pixel);
//        // and ends with a random color
//        RgbColor targetColor = RgbColor(random(peak), random(peak), random(peak));
//        // with the random ease function
//        AnimEaseFunction easing;
//
//        switch (random(3))
//        {
//        case 0:
//            easing = NeoEase::CubicIn;
//            Serial.println();
//            Serial.println("case 0: CubicIn");
//            break;
//        case 1:
//            easing = NeoEase::CubicOut;
//            Serial.println();
//            Serial.println("case 1: CubicOut");
//            break;
//        case 2:
//            easing = NeoEase::QuadraticInOut;
//            Serial.println();
//            Serial.println("case 2: QaudraticInOut");
//            break;
//        }
//
//#ifdef ARDUINO_ARCH_AVR
//        // each animation starts with the color that was present
//        animationState[pixel].StartingColor = originalColor;
//        // and ends with a random color
//        animationState[pixel].EndingColor = targetColor;
//        // using the specific curve
//        animationState[pixel].Easeing = easing;
//
//        // now use the animation state we just calculated and start the animation
//        // which will continue to run and call the update function until it completes
//        animations.StartAnimation(pixel, time, AnimUpdate);
//#else
//        // we must supply a function that will define the animation, in this example
//        // we are using "lambda expression" to define the function inline, which gives
//        // us an easy way to "capture" the originalColor and targetColor for the call back.
//        //
//        // this function will get called back when ever the animation needs to change
//        // the state of the pixel, it will provide a animation progress value
//        // from 0.0 (start of animation) to 1.0 (end of animation)
//        //
//        // we use this progress value to define how we want to animate in this case
//        // we call RgbColor::LinearBlend which will return a color blended between
//        // the values given, by the amount passed, hich is also a float value from 0.0-1.0.
//        // then we set the color.
//        //
//        // There is no need for the MyAnimationState struct as the compiler takes care
//        // of those details for us
//        AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
//        {
//            // progress will start at 0.0 and end at 1.0
//            // we convert to the curve we want
//            float progress = easing(param.progress);
//
//            // use the curve value to apply to the animation
//            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, targetColor, progress);
//            strip.SetPixelColor(pixel, updatedColor);
//        };
//
//        // now use the animation properties we just calculated and start the animation
//        // which will continue to run and call the update function until it completes
//        animations.StartAnimation(pixel, time, animUpdate);
//#endif
//    }
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//void loop()
//{
//    if (animations.IsAnimating())
//    {
//        // the normal loop just needs these two to run the active animations
//        animations.UpdateAnimations();
//        strip.Show();
//    }
//    else
//    {
//        Serial.println();
//        Serial.println("Setup Next Set...");
//        // example function that sets up some animations
//        SetupAnimationSet();
//    }
//}


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
