#include "UsermodMirrorChase.h"

// Forward declaration of effect function
static uint16_t mode_mirror_chase(void);

void UsermodMirrorChase::setup() {
  // Register the custom effect with WLED
  // Using 255 as ID means WLED will assign the next available ID dynamically
  if (!initDone) {
    strip.addEffect(255, &mode_mirror_chase, _data_fx);
    initDone = true;
  }
}

void UsermodMirrorChase::loop() {
  // No loop processing needed for this usermod
}

// Static strings
const char UsermodMirrorChase::_name[] PROGMEM = "Mirror Chase";

// Effect metadata: "Name@Speed,Width;Colors"
// Parameters:
//   Speed: Controls animation speed (lower = slower)
//   Width (Intensity): Band width in pixels (1-16 pixels per color)
// Segment settings (not in effect UI, use segment configuration):
//   Offset: Shifts center position from middle
//     - offset=0 → center at segment middle ((SEGLEN-1)/2)
//     - offset=2 → center shifted 2 pixels right from middle
//     - Works with any segment length (not limited to 255)
//   Reverse: Changes animation direction
//     - unchecked = outward flow from center (default)
//     - checked = inward flow toward center
// Colors: Uses 3 solid colors (color1, color2, color3)
const char UsermodMirrorChase::_data_fx[] PROGMEM = "Mirror Chase@!,Width;1,2,3";

/*
 * Mirror Chase Effect Implementation
 *
 * Creates a 3-color chase pattern that emanates from a center point.
 * Both sides of the center show the same colors at equidistant positions.
 * No wrapping in the chase pattern - the strip is treated as purely linear.
 */
static uint16_t mode_mirror_chase(void) {
  // Get parameters
  const uint8_t speed = SEGMENT.speed;
  const uint8_t intensity = SEGMENT.intensity;
  // Use segment's reverse setting to control animation direction
  const bool reverseDirection = SEGMENT.reverse;

  // Calculate center pixel position
  // offset=0 means center at middle, offset=N shifts center by N pixels from middle
  const uint16_t defaultCenter = (SEGLEN - 1) / 2;
  const uint16_t centerPixel = (defaultCenter + SEGMENT.offset) % SEGLEN;

  // Calculate band width (1-16 pixels per color)
  const unsigned width = 1 + (intensity >> 4);

  // Calculate animation timing (same pattern as Chase 3)
  const uint32_t cycleTime = 50 + ((255 - speed) << 1);
  const uint32_t it = strip.now / cycleTime;  // animation iterator

  // Total pattern length is 3 * width (3 colors cycling)
  const unsigned patternLength = width * 3;

  // Calculate maximum distance from center to know when to stop
  const uint16_t maxDistance = max(centerPixel, (uint16_t)(SEGLEN - 1 - centerPixel));

  // Optimization: Loop through distances instead of all pixels
  // Since the pattern is symmetric, we calculate color once per distance
  // and set pixels on both sides of center simultaneously
  for (uint16_t distance = 0; distance <= maxDistance; distance++) {
    // Determine position in the chase pattern based on distance from center
    unsigned patternPos;
    if (reverseDirection) {
      // Reverse mode: pattern chases inward toward center
      // Pixels further from center show "future" pattern positions
      patternPos = (it + distance) % patternLength;
    } else {
      // Normal mode: pattern chases outward from center
      // Pixels further from center show "past" pattern positions
      // Instead of (it - distance) which can be negative, we use the mathematical
      // equivalence: going back by N is the same as going forward by (period - N)
      patternPos = (it + (patternLength - distance)) % patternLength;
    }

    // Determine color based on pattern position (calculated once per distance)
    uint32_t color;
    if (patternPos < width) {
      color = SEGCOLOR(0);
    } else if (patternPos < width * 2) {
      color = SEGCOLOR(1);
    } else {
      color = SEGCOLOR(2);
    }

    // Set pixels at this distance from center
    if (distance == 0) {
      // Center pixel (only one pixel at distance 0)
      int16_t pixelPos = centerPixel;

      // Compensate for automatic offset and reverse
      int pixelIndex = pixelPos - (int)SEGMENT.offset;
      if (pixelIndex < 0) pixelIndex += SEGLEN;
      if (reverseDirection) pixelIndex = (SEGLEN - 1) - pixelIndex;

      SEGMENT.setPixelColor(pixelIndex, color);
    } else {
      // Left pixel (center - distance)
      int16_t leftPos = centerPixel - distance;
      if (leftPos >= 0) {
        int pixelIndex = leftPos - (int)SEGMENT.offset;
        if (pixelIndex < 0) pixelIndex += SEGLEN;
        if (reverseDirection) pixelIndex = (SEGLEN - 1) - pixelIndex;
        SEGMENT.setPixelColor(pixelIndex, color);
      }

      // Right pixel (center + distance)
      int16_t rightPos = centerPixel + distance;
      if (rightPos < SEGLEN) {
        int pixelIndex = rightPos - (int)SEGMENT.offset;
        if (pixelIndex < 0) pixelIndex += SEGLEN;
        if (reverseDirection) pixelIndex = (SEGLEN - 1) - pixelIndex;
        SEGMENT.setPixelColor(pixelIndex, color);
      }
    }
  }

  return FRAMETIME;
}

// Create a static instance and register it
static UsermodMirrorChase usermod_mirror_chase;
REGISTER_USERMOD(usermod_mirror_chase);
