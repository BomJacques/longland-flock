# Longland Flock development notes

## Version 0.4: ordered Brain and radar instrument

Removed the product Standalone format and executable from the current package. Windows output is VST3 only; developer verification executables are opt-in with `FLOCK_BUILD_TESTS=ON`. Historical ZIPs retain their original contents.

The processor now runs an ordered nine-stage Brain: the seven original rules, Scatter and Echo. A later steering contribution attenuates part of the preceding accumulator before adding its own bounded force, making composition noncommutative even below the acceleration cap. Scatter applies outward force boosted by energy/transient activity. Echo captures a bounded predicted position from steering accumulated at its position in the chain; it also applies propulsion and energy from measured per-tap echo activity. The next audio control frame converts those captured coordinates into time/pan targets. Freeze holds this ecological state; manual sound controls and audio tails remain active.

An integer parameter encodes one of 9! permutations, so a UI reorder or host automation event publishes a complete valid order in one atomic parameter write. No collection is edited concurrently with audio. Module enables remain independent, smoothed where applicable. Echo removal smoothly gates the audible delay level/send. All previous parameter IDs/indices remain, with five new parameters appended. Older orderless states use the default chain; sound changes relative to the old weighted-sum model are intentional.

The editor is split into `Editor.h`: original procedural/vector hardware styling, 15 main rotary dials, per-stage strength dials, drag/drop and button reordering, remove/add controls, and a circular phosphor radar. A cached background and 30 Hz visual updates keep UI work off the audio thread. The radar sweep is decorative; boid positions, trails, echo activity, capture-position links and tap readouts come from the simulation/audio snapshots. Viewport scaling works from 992x656 through 1860x1230 with a fixed aspect ratio. No third-party artwork or extra UI framework is used.

Verification: all 362,880 permutation round-trips pass. Moving Scatter produces different trajectories; adding it changes the controlled-test population spread from 0.271 to 0.978. Moving Echo across Scatter with otherwise identical audio settings produces a relative output difference of 1.397, proving it changes processing. Tests cover GUI reorder callbacks including drag coordinates, remove/re-enable, resizing, parameter/order state restoration, all previous sample-rate/buffer tests and actual VST3 discovery/load/audio/editor. Full and compact nine-module renders were visually inspected. Final offline processing remains approximately 4.8% of one core including harness overhead; this is not a worst-case callback deadline measurement. Third-party DAW listening and live pointer interaction in a host remain outside this automated verification.

Audition renders include identical-source, identical-setting comparisons with only Scatter/Echo order swapped. Use these to judge musical usefulness, not just measurable difference.

## Architecture

Eight fractional delay lines form a damped FDN. Version 0.2 uses three butterfly stages of paired rotations followed by a cyclic permutation. This matrix remains orthogonal at every diffusion setting. Rotation coefficients update every 64 samples from smoothed diffusion; trigonometric work stays out of the per-sample loop. Per-line RT60 gain stays below unity. A loop DC blocker and bounded soft saturation provide additional protection. Delay slew is restricted to 0.002 samples/sample, deliberately making large size changes gradual.

64 deterministic three-dimensional boids advance at 100 Hz. Neighbour forces use the previous population state, avoiding update-order bias. Audio analysis supplies energy, transient strength and a cheap brightness approximation. Boids control aggregate reverb parameters and, from version 0.3, individual read taps in a shared stereo delay recording.

The editor reads complete fixed-size snapshots over an SPSC queue. It cannot block the audio thread. Rule enable/strength controls are host parameters; visual overlays are editor-only. Freeze stops ecological movement, not the audio feedback network.

## Mapping intent

Spread controls width; neighbourhood density controls diffusion; mean Y changes delay scale; mean Z influences damping; average speed and velocity dispersion influence modulation. Smoothed aggregate motion drives delay trajectories. Behaviour strengths change those statistics indirectly.

## Tradeoffs / listening work

Large size changes settle slowly to limit pitch shifts. The displayed decay is nominal: damping, interpolation and soft saturation can shorten the measured tail. Diffusion no longer introduces matrix-blending losses. No infinite audio freeze is provided. Initial neighbour search is O(64 squared), pending profiling. Software tests do not replace DAW listening, host automation or device compatibility checks.

## Later experiments

Audition reflection density versus clarity on the supplied drum and pad renders; tune transient sensitivity and spectral response. Add spatial partitioning only if measured CPU justifies it. Velocity variance is a possible future mapping; the current implementation uses mean speed and centre position.

## Phase 2: decay and auditioning

Measured the original diffusion blend before changing it. A 0.1-amplitude impulse at 48 kHz, Size 1, Decay 6 seconds, Damping 0.2, Width 1 and Motion 0 was rendered after one second of settling, then analysed using backward energy integration and a least-squares -5 to -25 dB fit extrapolated to RT60. Sixteen seconds of output were captured. Broadband measurement includes filtering/interpolation losses and is not a promise of frequency-independent RT60.

| Diffusion | Version 0.1 measured RT60 | Version 0.2 measured RT60 |
|---|---:|---:|
| 0.60 | 0.656 s | 4.870 s |
| 0.80 | 1.038 s | 4.855 s |
| 0.95 | 2.518 s | 4.856 s |

Regression tests check matrix orthogonality/energy preservation over 101 settings and decay consistency across the measured settings. Preset parameter values and IDs remain compatible, but existing presets deliberately sound longer and fuller with the new feedback topology. The version 0.1 ZIP remains available as a baseline.

`FlockIntegration --audition <absolute-directory>` generates identical-source dry/processed drum and pad WAVs plus a listening guide with peak and tail-level measurements. Rendering performs finite/peak checks and applies no loudness normalisation. Files run for 14 seconds: six seconds of source, eight seconds of tail.

## Version 0.3 boid delay

Added `BoidDelay`: 64 read taps, two preallocated 3.2-second buffers. Alternate agents read left/right input, then pan their echoes across stereo. Y maps to centre delay time times `(1 + 0.9 * spread * y)`, bounded to 10–2850 ms. X sets equal-power pan; energy sets audible gain from 0.35 to 1, divided by 64. Pan and gain smooth over 25 ms. Small timing changes slew at at most 0.003 samples/sample; errors above 20 ms start a 50 ms convex dual-read crossfade. These are 64 taps, not 64 independently allocated feedback engines.

Feedback averages all tap samples with fixed 1/64 weights, independently of audible gain/pan, and passes through a low-pass, unity-bounded DC blocker and a maximum 0.85 loop gain. Convex interpolation and read crossfades do not amplify the return. Saturation bounds every buffer write. Allocation occurs only on prepare; the processing path uses existing buffers and fixed arrays. GUI echo data travels through the existing SPSC snapshot queue.

The combined wet signal is `ReverbLevel * reverb(input + DelaySend * DelayLevel * echoes) + DelayLevel * echoes`, then the existing wet protection and global Mix. Original parameter ordering and IDs are preserved; six parameters are appended. Legacy XML without these parameters receives DelayLevel 0 and ReverbLevel 1. Freeze stops the ecosystem, not delay repeats. A conservative 160-second tail is reported to hosts for long delay feedback plus reverb.

Dedicated delay tests verify first-arrival timing, 64 distinct taps, pan, abrupt time/spread automation, maximum-feedback decay and clearing on prepare at all three sample rates. A fractional-wrap test exposed float roundoff that could turn a tiny negative read position into `capacity`; both FDN and echo readers now calculate wrapped positions in double precision. The readable controls remain float-valued.

The selected-boid Level readout is its smoothed relative energy gain before division by 64, pan, global Delay level and Mix. The time readout is approximate during a two-head crossfade. Echo controls use milliseconds; host tempo sync and separate per-agent feedback loops are not implemented.

## Verified prototype results

Windows x64 Release VST3 and standalone builds succeed with JUCE 9.0.0 and MSVC 19.51. The compiled VST3 was discovered and loaded in a JUCE host harness; audio processing, state round-trip and editor creation passed. Processor tests cover 44.1/48/96 kHz, 32/128/512/1024-sample blocks, seven presets/stimuli, extreme controls, non-finite input, mono layout, transparent dry operation and repeated editor reopening. Buffer partitions of 32 and 511 samples produce identical rendered audio. Independent tests cover decaying impulses, extreme delay/filter/diffusion automation, zero/max rule weights, goal convergence, freeze and distinct transient versus sustained flock responses.

Final aggregate offline run processed 84.31 audio seconds in 0.83 wall seconds on this machine (about 0.98% of one core, including test overhead). This measures throughput, not worst-case callback timing. Audio-thread allocation/lock safety and the SPSC snapshot design were reviewed by inspection. No third-party DAW, audio hardware listening, AU build or race-detector run was performed.

Version 0.3 final integration run: 84.31 simulated audio seconds in 3.91 wall seconds (4.64% aggregate single-core throughput including checks). Delay-only routing produces the expected first echo at 100 ms; legacy states restore with echo level off. Actual VST3 load/audio/state/editor checks pass. This does not establish worst-case realtime callback deadlines.

