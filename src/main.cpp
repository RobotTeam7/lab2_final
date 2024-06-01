#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <vector>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible
#define PI 3.14159265358979323846
Adafruit_SSD1306 display_handler(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned long sample_time_ms = 40; 

#define READ_PIN B1

void setup() {
  pinMode(READ_PIN, INPUT);

  display_handler.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 
  // Displays Adafruit logo by default. call clearDisplay immediately if you don't want this.
  display_handler.display();

  delay(2000);

  // Displays "Hello world!" on the screen
  display_handler.clearDisplay();
  display_handler.setTextSize(1);
  display_handler.setTextColor(SSD1306_WHITE);
  display_handler.setCursor(0,0);
  display_handler.println("Hello world!");
  display_handler.display();

}

// CHATGPT cooked here
// Seems good to me, but I haven't looked in too much detail - F
void generateSineWave(int num_samples, double frequency, double total_time_ms, std::vector<double>& sine_wave) {
    double amplitude = 1.0; // Amplitude of the sine wave
    double phase = 0.0; // Phase shift in radians
    double total_time_s = total_time_ms / 1000.0; // Convert milliseconds to seconds
    double sample_rate = (double) num_samples / total_time_s; // Sample rate in Hz

    sine_wave.resize(num_samples); // Resize the vector to hold the required number of samples

    for (int i = 0; i < num_samples; i++) {
      double t = (double)i / sample_rate; // Time at this sample in seconds
      double value = amplitude * sin(2.0 * PI * frequency * t + phase);
      sine_wave[i] = value;
    }
}

double convolution(int frequency, int sample_number, double* signal) { 
  
  // The reference wave will have twice the lengh of the signal. Thus allowing us to test
  // every possible offset value. This gives us the highest chance of finding the point
  // where the two waves are perfectly in phase, but is probably overkill.
  std::vector<double> reference_wave(sample_number * 2);
  generateSineWave(sample_number * 2, frequency, sample_time_ms * 2, reference_wave);
    
  double values_per_offset[sample_number];
  for(int offset = 0; offset < sample_number; offset++) {
    double summation = 0;
    for(int index = 0; index < sample_number; index++) {
      summation += reference_wave[offset + index] * signal[index];
    }
    values_per_offset[offset] = summation;
  }

  double maxValue = 0;
  for(int i = 0; i < sample_number; i++) {
    if(values_per_offset[i] > maxValue) {
      maxValue = values_per_offset[i];
    }
  }
  return maxValue;
}

void loop() {
  
  // Signal Array. Assumes we won't be taking more then 500 samples
  // Switching to a vector might be more memory efficient - F
  static double signal[500];


  // Sampling for the given time frame
  int num_samples = 0;
  unsigned long start_time = millis();
  while(millis() - start_time < sample_time_ms) {
    signal[num_samples] = analogRead(READ_PIN);
    num_samples++;

    // Introducing a delay here may make our sample time more consistant
    // Since we are sampling many periods, the consistancy of our sample rate is more important
    // than sampling speed.
    delayMicroseconds(500);
  }

  double convolution_1000Hz = convolution(1000, num_samples, signal);
  
  display_handler.clearDisplay();
  display_handler.setCursor(0,0);
  display_handler.print(convolution_1000Hz);
  display_handler.display();
  num_samples = 0;

  // delay for a bit here so that the display is readable
  delay(100);
}