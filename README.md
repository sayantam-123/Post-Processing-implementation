# Post-Processing-implementation
  Screen-space post processing pipeline in Raylib — multi-pass GLSL shader effects chained over an offscreen render texture.

## Effects
- Sobel edge detection
- Grayscale (luminance)
- Negative
- Predator vision
- Laplacian sharpening
- Film grain
- Motion blur (camera, depth-buffer based)

## Controls
- `F` — cycle through post-process effects

## Tweaking Motion Blur

In `motion_blur.cpp`:
- `samples` — number of blur samples, higher = smoother but slower (default: 16)
