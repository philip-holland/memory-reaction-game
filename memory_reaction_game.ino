#include <Adafruit_CircuitPlayground.h>

/*
 * Memory/Reaction Game - 20210924 - @philipholland
 * - Remember the neopixel position shown in green
 * - Press any button to stop the white neopixel cursor on it as it circles
 * - Green/red indication of correctness
 * - Cursor moves faster for each correct round
 * - Slide switch mutes sound effects
 * 
 */


#define PLAY_PIXEL_ON  0xFFFFFF   // white
#define PLAY_PIXEL_OFF 0x000000   // black
#define FRAME_START_INCR 60       // milliseconds, between frames
#define FRAME_SPEEDUP 5           // speedup per round
#define UPDATE_FRAME 10           // update frame, every Nth frame
#define PIXEL_WIN 0x00FF00        // win colour, green
#define PIXEL_LOSE 0xFF0000       // lose colour, red
#define PREVIEW 3                 // no. of flashes of the preview
#define PREVIEW_PAUSE 300         // duration of flashes
#define POST_GAME_PAUSE 1000      // milliseconds, after game is finished                           
#define NUM_PIXELS 10             // neopixel count
                                  
// freq for sound fx
#define NOTE_Bf4 466
#define NOTE_A4 440
#define NOTE_Af4 415
#define NOTE_G4 392
#define NOTE_C5 523
#define NOTE_C6 1047

// global, to survive loop()
int FRAME_INCR = FRAME_START_INCR; 
int SCORE=0;

void setAllPixelsColor(uint32_t color) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    CircuitPlayground.setPixelColor(i, color);
  }
}


void playToneOrWait(int tone, int duration) {
  if (CircuitPlayground.slideSwitch()) {
    CircuitPlayground.playTone(tone, duration);
  } else {
    delay(duration);
  }
}

void checkSolution(int solution, int attempt) {
  // white backdrop
  setAllPixelsColor(PLAY_PIXEL_ON);
  // show the solution
  CircuitPlayground.setPixelColor(solution, PIXEL_WIN);

  // show incorrect attempt (as needed) and ready for next round (score, speed)
  if (solution != attempt) {
    CircuitPlayground.setPixelColor(attempt, PIXEL_LOSE);
    FRAME_INCR = FRAME_START_INCR; // reset game speed
    SCORE = 0;
  } else {
    SCORE++;
    FRAME_INCR -= FRAME_SPEEDUP; // speed-up for next round (bounded)
    FRAME_INCR = (FRAME_INCR <= 0) ? 10 : FRAME_INCR;
  }

  delay(POST_GAME_PAUSE*2);

  if (solution == attempt) {  // win: all pixels green, sound: beeps your score
    setAllPixelsColor(PIXEL_WIN);
    for (int i=0; i<SCORE; i++) {
      playToneOrWait(NOTE_C6, PREVIEW_PAUSE / 2);
      delay(PREVIEW_PAUSE / 4);
    }
    
  } else {                   // lose: all pixels red, sound: wa-wa-wa-waaah
    setAllPixelsColor(PIXEL_LOSE);
    playToneOrWait(NOTE_Bf4, PREVIEW_PAUSE); 
    playToneOrWait(NOTE_A4, PREVIEW_PAUSE); 
    playToneOrWait(NOTE_Af4, PREVIEW_PAUSE); 
    playToneOrWait(NOTE_G4, PREVIEW_PAUSE * 2); 
  }

  delay(POST_GAME_PAUSE);

}

// show the solution for the player to target
void previewSolution(int solution) {
  CircuitPlayground.clearPixels();
  // show the goal
  for (int i = 0; i < PREVIEW; i++) {
    CircuitPlayground.setPixelColor(solution, PIXEL_WIN);
    playToneOrWait(NOTE_C5, PREVIEW_PAUSE);
    CircuitPlayground.setPixelColor(solution, PLAY_PIXEL_OFF);
    delay(PREVIEW_PAUSE);
  }
  playToneOrWait(NOTE_C6, PREVIEW_PAUSE * 1.5);
}

void setup()
{
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(10);
  CircuitPlayground.clearPixels();

  // add some entropy from sensors
  randomSeed(CircuitPlayground.lightSensor() + CircuitPlayground.temperature() + CircuitPlayground.soundSensor());
}

void loop()
{
  int pixelIndex = -1;
  int solutionPixel = random(0, NUM_PIXELS);
  int updateFrame;
  boolean rightButton;

  // start with a preview
  previewSolution(solutionPixel);

  // game loop
  while (true) {

    // update game field every N frames ...
    if (++updateFrame >= UPDATE_FRAME) {
      updateFrame = 0;

      // change active cursor pixel
      if (++pixelIndex > (NUM_PIXELS-1)) {
        pixelIndex = 0;
      }

      CircuitPlayground.setPixelColor(pixelIndex, PLAY_PIXEL_ON);
      CircuitPlayground.setPixelColor(pixelIndex == 0 ? (NUM_PIXELS-1) : pixelIndex - 1, PLAY_PIXEL_OFF);

    }

    // ... but update button press status each frame
    rightButton = CircuitPlayground.rightButton() || CircuitPlayground.leftButton();

    // check for correct solution, if button pressed
    if (rightButton) {
      checkSolution(solutionPixel, pixelIndex);
      break;
    }

    // wait for next frame
    delay(FRAME_INCR);

  }

}
