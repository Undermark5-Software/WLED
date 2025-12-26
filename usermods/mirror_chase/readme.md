# Mirror Chase Effect

A custom WLED effect that creates a mirrored chase pattern emanating from a user-defined center point.

## Features

- **Mirrored chase pattern** - Colors flow outward (or inward) from a configurable center point
- **No wrapping artifacts** - The strip is treated as linear, not circular
- **Perfect center alignment** - Pixels at equal distances from center always show matching colors
- **Configurable parameters**:
  - Speed: Animation speed
  - Width: Band width (1-16 pixels per color)
  - Offset: Adjust center position
  - Reverse: Toggle between outward and inward flow
- **3 solid colors** - Uses WLED's standard 3-color system (no palette/gradient)

## Installation

### Method 1: Using custom_usermods (Recommended)

Add to your `platformio_override.ini`:

```ini
[env:my_board]
extends = env:esp32dev  ; or your target board
build_flags = ${common.build_flags} ${esp32.build_flags}
  -D USERMOD_MIRROR_CHASE
custom_usermods = mirror_chase
```

### Method 2: Manual build flag

Add to your `platformio_override.ini`:

```ini
build_flags =
  ${common.build_flags}
  -D USERMOD_MIRROR_CHASE
```

The usermod will be automatically discovered via the `library.json` file.

## Usage

1. After flashing, go to the WLED web interface
2. Select the "Mirror Chase" effect from the effects list
3. **Effect sliders** (in effect UI):
   - **Speed slider**: Controls animation speed (higher = faster)
   - **Width slider**: Controls the width of each color band (1-16 pixels)
4. **Segment settings** (in segment configuration, not effect sliders):
   - **Offset**: Adjusts the center position
     - Offset = 0: Center at physical middle of strip
     - Offset = 1, 2, 3...: Center shifted right (forward) by N pixels
     - Offset = -1, -2, -3...: Center shifted left (backward) by N pixels (web UI auto-converts negative values)
     - Works with any segment length (not limited to 255 pixels)
   - **Reverse checkbox**: Toggle between outward chase (default) and inward chase
5. Choose your 3 colors using the color pickers (Color 1, Color 2, Color 3)

## How It Works

### Center Position with Offset

The effect uses the segment's offset value to determine the center point:

- **Default center** = Middle of segment ((SEGLEN-1)/2)
- **Adjusted center** = ((SEGLEN-1)/2 + offset) % SEGLEN

This formula ensures perfect symmetry:
- **Odd-length strips**: Center lands on the exact middle pixel
- **Even-length strips**: Center lands on the left-of-center pixel for perfect mirroring

Examples:
- For a 79-pixel strip (0-78):
  - Offset 0 → Center at pixel 39 (middle)
  - Offset 1 → Center at pixel 40 (one pixel right)
  - Offset 2 → Center at pixel 41 (your gable peak!)
  - Offset -1 → Center at pixel 38 (one pixel left)
  - Offset -2 → Center at pixel 37 (two pixels left)
  - Offset 39 → Center at pixel 78 (end)
  - Offset 40 → Center at pixel 0 (wraps to beginning)
- For a 16-pixel strip (0-15):
  - Offset 0 → Center at pixel 7 (left-of-center for perfect symmetry)
  - Offset 1 → Center at pixel 8 (right-of-center)

### Pattern Calculation

The effect uses an optimized algorithm that takes advantage of the symmetric pattern:

1. **Loop through distances** (not individual pixels) - roughly half the iterations
2. For each distance from center:
   - Calculate the color once
   - Set pixels on both sides of center simultaneously (left and right)
3. **Elegant modular arithmetic** for direction:
   - Outward: `(time + (period - distance)) % period` - no negative numbers!
   - Inward: `(time + distance) % period`
4. **No wrapping artifacts** - the chase pattern is purely linear, not circular

This optimization provides:
- ~50% fewer iterations compared to looping through all pixels
- ~50% fewer color calculations
- Cleaner mathematical approach without magic constants

### Parameters

| Parameter | Control | Location | Range | Description |
|-----------|---------|----------|-------|-------------|
| Speed | Speed slider | Effect UI | 0-255 | Animation speed (lower = slower) |
| Width | Width slider | Effect UI | 0-255 | Maps to 1-16 pixels per color band |
| Offset | Offset setting | Segment config | 0 to SEGLEN-1 | Shifts center position from middle |
| Reverse | Reverse checkbox | Segment config | On/Off | Reverses chase direction (inward vs outward) |

## Example Use Cases

### Symmetric Gable Peak (79 pixels, center at pixel 41)

1. Set segment offset to 2 (shifts from pixel 39 to pixel 41)
2. Adjust width for desired band size
3. Choose complementary colors
4. The chase will flow outward from pixel 41 symmetrically

### Dynamic Center Animation

1. Create multiple presets with different offset values
2. Transition between presets to "move" the chase center
3. Creates the illusion of a traveling origin point

## Technical Details

- **Effect ID**: Dynamically assigned (uses 255 for auto-assignment)
- **Usermod ID**: 55 (`USERMOD_ID_MIRROR_CHASE`)
- **Frame delay**: FRAMETIME (~24ms, 42 FPS)
- **Colors**: Uses SEGCOLOR(0), SEGCOLOR(1), and SEGCOLOR(2) (3 solid colors, no palette)
- **Performance**: Optimized to loop through distances (~50% fewer iterations than pixel-by-pixel)
- **WLED Integration**:
  - Compensates for WLED's automatic `offset` addition in `setPixelColor()`
  - Compensates for WLED's automatic `reverse` transformation in `setPixelColor()`
  - Uses segment's built-in offset/reverse settings instead of custom sliders

## Differences from Chase 3

| Feature | Chase 3 | Mirror Chase |
|---------|---------|--------------|
| Pattern flow | Circular (wraps around) | Linear from center |
| Center position | Fixed | User-configurable via offset |
| Mirroring | Optional, but causes wrapping artifacts | Built-in, no wrapping |
| Edge alignment | Always aligned | May vary (center takes priority) |

## Troubleshooting

**Q: The center doesn't seem to be where I expect**
- A: Remember that offset=0 means center at (SEGLEN-1)/2. For a 79-pixel strip, that's pixel 39. For even-length strips, it picks the left-of-center pixel.

**Q: Where do I set the offset and reverse?**
- A: These are **segment settings**, not effect sliders. Click on the segment configuration (gear icon or segment name) to access offset and reverse settings.

**Q: Colors don't match at the edges**
- A: This is expected behavior. The effect prioritizes perfect centering, so edges may show different colors.

**Q: The reverse checkbox doesn't work**
- A: Make sure you're toggling the **segment's** reverse setting, not looking for an effect checkbox. The effect uses the segment's built-in reverse feature.

**Q: Effect not showing in WLED**
- A: Ensure USERMOD_MIRROR_CHASE is defined in your build flags and you've recompiled/reflashed.

## Credits

Created for WLED custom installation with asymmetric pixel count requiring centered chase patterns.
