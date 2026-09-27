# Longland Flock 0.4 — ordered Brain / radar edition

Windows x64 **VST3 effect**. Standalone application generation has been removed and no standalone executable is included in this release.

## Load it

Copy the entire `Binaries/Longland Flock.vst3` bundle into your DAW's VST3 location (normally `C:\Program Files\Common Files\VST3`) and rescan. Insert Longland Flock on an audio track. It processes incoming audio; it is not an instrument. The VST3 has been loaded and tested in a JUCE host harness. Audition in your own DAW before committing a session.

## The ordered Boid Brain

The rack runs from top to bottom. Drag a row by its grip/body, or use its up/down buttons. The strength dial sets how much that stage acts. VIEW selects its radar overlay. The x button removes a module from the active chain; + MODULE adds it again. There is one instance of each module, up to nine stages. Removed modules retain their stored position and strength.

**Order changes the effect.** A later steering stage reshapes the earlier accumulated steering. For example, putting Scatter late can spread out a flock that Flock gathered; putting Flock after Scatter can pull that motion back together. Order is a single host-automatable parameter and is included in saved plugin state.

| Module | Behaviour |
|---|---|
| Flock | Gathers nearby agents |
| Separate | Pushes close neighbours apart |
| Align | Smooths their headings |
| Goal | Pulls toward the G target |
| Avoid | Avoids the A region and transient repulsion |
| Follow | Follows the leader |
| Wander | Adds smooth individual variation |
| Scatter | Pushes outward; stronger with input energy and transients |
| Echo | Captures tap positions at this point in the chain and returns echo activity to boid energy |

### Echo belongs to the Brain

Echo is a control stage for the existing 64-tap audio delay, not a second copy of it. Its rack dial controls **Echo brain drive**: how strongly current steering predicts tap positions and how strongly audible echoes re-excite the agents. Moving Echo earlier captures less of the later steering in that frame. Moving it after Scatter captures the scattering impulse immediately. Subsequent frames continue to reflect the flock's evolving physical state.

Removing Echo fades its audible delay and reverb send out; the reverb remains available. Internal delay memory continues running, so re-enabling it may reveal remaining repeats. Freeze holds ecological movement, while audio delay/reverb tails continue. Manual audio controls remain usable.

## Hardware-style controls

Fifteen rotary controls are grouped into SWARM, ECHO and SPACE. Drag a dial to turn it, double-click to reset, or click its digital value to type a value. Percentages use 0–100 for a normalised range; Energy and Width may exceed 100%. Delay time uses milliseconds, Decay seconds and Size a scale factor. Resize the window proportionally from 80% to 150%.

- **SWARM:** Energy, transient Reaction, reverb Motion and neighbour Radius.
- **ECHO:** Time (20–1500 ms centre time), Spread, Feedback (up to 85%), Level and Send to reverb.
- **SPACE:** Size, Decay, Damping, Width, Reverb level and global dry/wet Mix.

Set Reverb to 0 to hear only the boid delay. Set global Mix to 100% for wet only. Delay Spread maps Y position to up to ±90% of centre time; taps stay within 10–2850 ms. X controls echo pan. Energy controls each audible tap's level. Alternate boids read left/right input from shared buffers and pan those recordings across stereo. No tempo sync is included yet.

Global wet = `ReverbLevel * reverb(input + Send * DelayLevel * echoes) + DelayLevel * echoes`, followed by output protection. Feedback averages taps independently of their output level and pan. Large time changes crossfade over 50 ms; small movements glide.

## Radar

The green screen shows the actual three-dimensional flock projected onto XY. The sweep is a visual scanning cue; it does not drive the simulation or audio. Click a boid to select it. Drag G and A to move the goal and avoidance region. The screen identifies the selected rule, and the lower readout shows the selected agent's delay time, pan and relative level.

TRAIL, VECTOR, RULE, TARGET and TAPS toggle overlays only. TAPS draws a link from the selected boid's physical position to its Brain-captured echo position. During a delay crossfade, the time readout represents an approximate blend. The strip below the radar plots all 64 tap times from 0 to 3000 ms; click it to select a tap.

## First experiment

1. Start with Murmuration and short drum hits. Set ECHO Level to 100%, SPACE Reverb to 0 and Mix to 65%.
2. Add Scatter. Turn its strength up and watch the cloud open out.
3. Move Echo above Scatter, then below it. Compare the echo pattern while keeping the dials fixed.
4. Move Flock after Scatter to hear the competing stages reshape the motion.
5. Bring Reverb back in and raise Send to turn the echoes into a moving tail.

`Auditions/Listening-guide.md` links matched dry sources, full presets, delay-only examples and an order-only comparison: **Scatter then Echo** versus **Echo then Scatter**. All examples use the same corresponding source without loudness normalisation. They support listening; they do not substitute for it.

## State and compatibility

Original parameter IDs and ordering are preserved; new controls are appended. Older states without the delay controls load with Delay level off. States without Brain order use the default ordered chain. The ordered steering model deliberately changes the sound relative to 0.3 even when old strength values are restored. Factory presets include sensible Scatter/Echo choices. Exact particle trajectories, radar overlay choices and editor size are not stored.

## Build and tests

C++17, CMake 3.22+ and JUCE 9 are required. The configured local JUCE path is `C:/SDK/JUCE`; override with `-DJUCE_DIR=your/path`.

PowerShell 7, from this folder:

```powershell
./CMake.ps1 -BuildArgs @('-S', '.', '-B', 'build', '-G', 'Visual Studio 18 2026', '-A', 'x64')
./CMake.ps1 -BuildArgs @('--build', 'build', '--config', 'Release', '--parallel', '6')
```

The product target is `LonglandFlock_VST3`; AU is declared only on Apple platforms and has not been tested. Enable developer verification with `-DFLOCK_BUILD_TESTS=ON` when configuring. Developer-only test executables are FlockTests, FlockMeasure, DelayTests, BrainTests and FlockIntegration. They are not standalone product apps. FlockIntegration accepts a PNG output path plus a VST3 path, or `--audition <absolute-directory>` to render examples.

## Source map

- `Source/Plugin.cpp`: audio routing, parameters, state and control scheduling.
- `Source/Editor.h`: original vector-drawn hardware interface, dials and radar.
- `Source/BrainOrder.h`: atomic permutation encoding and stage moves.
- `Source/Swarm.h`: ordered steering, Echo capture and population statistics.
- `Source/BoidDelay.h`: 64-tap shared-buffer delay and echo-activity return.
- `Source/Engine.h`: eight-line orthogonal FDN reverb.
- `Source/*Tests.cpp`, `Measure.cpp`, `Integration.cpp`: automated verification.

Design references: [Arturia CS-80 V](https://www.arturia.com/products/software-instruments/cs-80v/overview) and [XILS-lab The Eighty manual](https://www.bhphotovideo.com/lit_files/1188940.pdf). The interface artwork, layout and controls are original JUCE vector drawing; no third-party UI artwork is embedded. JUCE remains a separate dependency under its own licence.

