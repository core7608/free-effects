#include "expression_library.h"
#include <algorithm>

namespace FreeEffect {

ExpressionLibrary& ExpressionLibrary::instance() {
    static ExpressionLibrary lib;
    return lib;
}

void ExpressionLibrary::loadBuiltins() {
    m_entries.clear();

    // Wiggle expressions
    m_entries.push_back({"Wiggle (Basic)", "Wiggle",
        "freq * Math.sin(time * freq * 2 * Math.PI) * amp",
        "Basic sinusoidal wiggle with frequency and amplitude"});

    m_entries.push_back({"Wiggle (Random)", "Wiggle",
        "wiggle(freq, amp)",
        "Random wiggle with frequency (Hz) and amplitude"});

    m_entries.push_back({"Wiggle (Smooth)", "Wiggle",
        "wiggle(freq, amp, 3)",
        "Smooth wiggle with 3 octaves for natural motion"});

    m_entries.push_back({"Wiggle (Position)", "Wiggle",
        "[wiggle(freq, amp), wiggle(freq * 0.7, amp * 0.7)]",
        "2D position wiggle with offset frequencies"});

    m_entries.push_back({"Wiggle (50% dampened)", "Wiggle",
        "amp * Math.sin(time * freq * 2 * Math.PI) * Math.exp(-time * damping)",
        "Dampened wiggle that fades over time"});

    m_entries.push_back({"Wiggle (Tempo Sync)", "Wiggle",
        "wiggle(comp.frameRate / bpm * 4, amp)",
        "Wiggle synchronized to music BPM"});

    m_entries.push_back({"Wiggle (Squared)", "Wiggle",
        "amp * (wiggle(freq, 1) * wiggle(freq, 1))",
        "Squared wiggle for sharper motion bursts"});

    m_entries.push_back({"Wiggle (One Axis)", "Wiggle",
        "[wiggle(freq, amp), value[1]]",
        "Wiggle only on X axis, preserve Y"});

    // Loop expressions
    m_entries.push_back({"Loop (Cycle)", "Loop",
        "loopOut(\"cycle\")",
        "Repeat keyframe cycle indefinitely"});

    m_entries.push_back({"Loop (Pingpong)", "Loop",
        "loopOut(\"pingpong\")",
        "Reverse keyframes at end for ping-pong effect"});

    m_entries.push_back({"Loop (Offset)", "Loop",
        "loopOut(\"offset\")",
        "Continue keyframe pattern with accumulated offset"});

    m_entries.push_back({"Loop (Continue)", "Loop",
        "loopOut(\"continue\")",
        "Continue motion at same rate past last keyframe"});

    m_entries.push_back({"Loop (Cycle Count)", "Loop",
        "loopOut(\"cycle\", n)",
        "Repeat keyframe cycle n times"});

    m_entries.push_back({"Loop (Infinite)", "Loop",
        "loopIn(\"cycle\") + loopOut(\"cycle\")",
        "Loop both before and after keyframe range"});

    m_entries.push_back({"Loop (Time)", "Loop",
        "t = time % duration; linear(t, 0, duration, startVal, endVal)",
        "Time-based looping with linear interpolation"});

    m_entries.push_back({"Loop (Pingpong Smooth)", "Loop",
        "amp = (endVal - startVal) / 2; mid = (startVal + endVal) / 2; mid + amp * Math.sin(time * Math.PI / duration)",
        "Smooth ping-pong using sine wave"});

    // Bounce / Overshoot
    m_entries.push_back({"Bounce (Basic)", "Bounce",
        "n = 0; if (time > 0) { n = 5; while (time - n / freq > 0 && n < 100) n++; } amp * Math.pow(-1, n) * Math.exp(-decay * n)",
        "Basic bounce with frequency and decay"});

    m_entries.push_back({"Bounce (Elastic)", "Bounce",
        "amp * Math.exp(-decay * time) * Math.sin(freq * time * 2 * Math.PI)",
        "Elastic bounce with exponential decay"});

    m_entries.push_back({"Bounce (Overshoot)", "Bounce",
        "s = 1.70158; t = time / duration - 1; endVal + amp * (t * t * ((s + 1) * t + s) + 1)",
        "Overshoot cubic easing"});

    m_entries.push_back({"Bounce (Spring)", "Bounce",
        "d = 0.7; freq = 3; endVal + amp * Math.exp(-d * time) * Math.sin(freq * time * 2 * Math.PI) / Math.exp(-d * 0)",
        "Spring physics bounce"});

    m_entries.push_back({"Bounce (Damped)", "Bounce",
        "amp * Math.exp(-damping * time) * Math.cos(frequency * time * 2 * Math.PI)",
        "Damped harmonic oscillator"});

    m_entries.push_back({"Bounce (Floor)", "Bounce",
        "y = v0 * time - 0.5 * g * time * time; if (y < 0) { y = 0; v0 = -v0 * restitution; } y",
        "Ball bouncing off floor with gravity"});

    // Spring
    m_entries.push_back({"Spring (Position)", "Spring",
        "target = [960, 540]; vel = [0, 0]; stiffness = 0.1; damping = 0.8; force = (target - value) * stiffness; vel = (vel + force) * damping; value + vel",
        "Spring physics toward target position"});

    m_entries.push_back({"Spring (Stiff)", "Spring",
        "force = (target - value) * 0.3; value + force * (1 - Math.exp(-time * 10))",
        "High stiffness spring"});

    m_entries.push_back({"Spring (Loose)", "Spring",
        "force = (target - value) * 0.02; value + force * (1 - Math.exp(-time * 2))",
        "Loose spring with slow settling"});

    // Easing functions
    m_entries.push_back({"Ease In (Cubic)", "Easing",
        "t = time / duration; endVal * t * t * t",
        "Cubic ease-in acceleration"});

    m_entries.push_back({"Ease Out (Cubic)", "Easing",
        "t = time / duration - 1; endVal * (t * t * t + 1)",
        "Cubic ease-out deceleration"});

    m_entries.push_back({"Ease In-Out (Cubic)", "Easing",
        "t = time / duration * 2; if (t < 1) endVal / 2 * t * t * t; else { t -= 2; endVal / 2 * (t * t * t + 2) }",
        "Cubic ease-in-out"});

    m_entries.push_back({"Ease In (Quadratic)", "Easing",
        "t = time / duration; endVal * t * t",
        "Quadratic ease-in"});

    m_entries.push_back({"Ease Out (Quadratic)", "Easing",
        "t = time / duration; endVal * t * (2 - t)",
        "Quadratic ease-out"});

    m_entries.push_back({"Ease In-Out (Sine)", "Easing",
        "t = time / duration; -(Math.cos(Math.PI * t) - 1) / 2 * endVal",
        "Sine ease-in-out"});

    m_entries.push_back({"Ease In (Expo)", "Easing",
        "t = time / duration; t === 0 ? 0 : endVal * Math.pow(2, 10 * (t - 1))",
        "Exponential ease-in"});

    m_entries.push_back({"Ease Out (Expo)", "Easing",
        "t = time / duration; t === 1 ? endVal : endVal * (1 - Math.pow(2, -10 * t))",
        "Exponential ease-out"});

    m_entries.push_back({"Ease In-Back", "Easing",
        "s = 1.70158; t = time / duration; endVal * t * t * ((s + 1) * t - s)",
        "Back ease-in with overshoot"});

    m_entries.push_back({"Ease Out-Back", "Easing",
        "s = 1.70158; t = time / duration - 1; endVal * (t * t * ((s + 1) * t + s) + 1)",
        "Back ease-out with anticipation"});

    // Random distributions
    m_entries.push_back({"Random (Uniform)", "Random",
        "random(min, max)",
        "Uniform random between min and max"});

    m_entries.push_back({"Random (Gaussian)", "Random",
        "gaussRandom(mean, stddev)",
        "Gaussian/normal distribution random"});

    m_entries.push_back({"Random (Seed)", "Random",
        "seedRandom(seed, true); random(min, max)",
        "Seeded reproducible random"});

    m_entries.push_back({"Random (Integer)", "Random",
        "Math.floor(random(min, max + 1))",
        "Random integer in range"});

    m_entries.push_back({"Random (From Array)", "Random",
        "arr = [1, 2, 3, 4, 5]; arr[Math.floor(random(arr.length))]",
        "Random element from array"});

    m_entries.push_back({"Random (Perlin)", "Random",
        "noise(time * freq) * amp",
        "Perlin noise-based smooth random"});

    // Math utilities
    m_entries.push_back({"Angle (To Target)", "Math",
        "Math.atan2(targetY - value[1], targetX - value[0]) * 180 / Math.PI",
        "Angle from position to target in degrees"});

    m_entries.push_back({"Distance (To Target)", "Math",
        "Math.sqrt(Math.pow(targetX - value[0], 2) + Math.pow(targetY - value[1], 2))",
        "Euclidean distance to target"});

    m_entries.push_back({"Linear (Interpolation)", "Math",
        "linear(time, tMin, tMax, valMin, valMax)",
        "Linear interpolation between values"});

    m_entries.push_back({"Clamp", "Math",
        "Math.min(max, Math.max(min, value))",
        "Clamp value between min and max"});

    m_entries.push_back({"Map Range", "Math",
        "val = (value - inMin) / (inMax - inMin); outMin + val * (outOut - outMin)",
        "Remap value from one range to another"});

    m_entries.push_back({"Abs", "Math",
        "Math.abs(value)",
        "Absolute value"});

    m_entries.push_back({"Smoothstep", "Math",
        "t = Math.max(0, Math.min(1, (value - edge0) / (edge1 - edge0))); t * t * (3 - 2 * t)",
        "Hermite smoothstep interpolation"});

    // Color expressions
    m_entries.push_back({"RGB to Hex", "Color",
        "function rgbToHex(r, g, b) { return '#' + [r, g, b].map(x => Math.round(x * 255).toString(16).padStart(2, '0')).join(''); }",
        "Convert RGB values to hex color string"});

    m_entries.push_back({"Hex to RGB", "Color",
        "r = parseInt(hex.slice(1, 3), 16) / 255; g = parseInt(hex.slice(3, 5), 16) / 255; b = parseInt(hex.slice(5, 7), 16) / 255",
        "Convert hex color to RGB values"});

    m_entries.push_back({"Brightness", "Color",
        "0.299 * value[0] + 0.587 * value[1] + 0.114 * value[2]",
        "Calculate luminance brightness"});

    m_entries.push_back({"Hue Shift", "Color",
        "h = (Math.atan2(value[2] - value[1], value[0] - value[2]) * 180 / Math.PI + shift + 360) % 360",
        "Shift hue by degrees"});

    m_entries.push_back({"Invert Color", "Color",
        "[1 - value[0], 1 - value[1], 1 - value[2]]",
        "Invert RGB color channels"});

    m_entries.push_back({"Mix Colors", "Color",
        "value * (1 - t) + target * t",
        "Linearly interpolate between two colors"});

    m_entries.push_back({"Grayscale", "Color",
        "lum = 0.299 * value[0] + 0.587 * value[1] + 0.114 * value[2]; [lum, lum, lum]",
        "Convert to grayscale using luminance weights"});

    // Time-based
    m_entries.push_back({"Time Loop", "Time",
        "t = time % loopDuration; t / loopDuration * endVal",
        "Loop time and map to output range"});

    m_entries.push_back({"Time Reverse", "Time",
        "endVal - (time / duration) * endVal",
        "Reverse time-based animation"});

    m_entries.push_back({"Frame to Time", "Time",
        "frame / comp.frameRate",
        "Convert frame number to time in seconds"});

    m_entries.push_back({"Time to Frame", "Time",
        "Math.floor(time * comp.frameRate)",
        "Convert time to frame number"});

    m_entries.push_back({"Time Warp", "Time",
        "time * speed + offset",
        "Scale and offset time"});

    // Spatial
    m_entries.push_back({"Orbit", "Spatial",
        "radius = 200; cx = 960; cy = 540; [cx + radius * Math.cos(time * speed), cy + radius * Math.sin(time * speed)]",
        "Circular orbit around center point"});

    m_entries.push_back({"Spiral", "Spatial",
        "r = baseRadius + time * growRate; angle = time * speed; [cx + r * Math.cos(angle), cy + r * Math.sin(angle)]",
        "Spiral path outward from center"});

    m_entries.push_back({"Wave (Sine)", "Spatial",
        "baseY + amplitude * Math.sin(time * frequency * 2 * Math.PI)",
        "Sinusoidal wave motion on Y axis"});

    m_entries.push_back({"Wave (Figure 8)", "Spatial",
        "a = 200; [cx + a * Math.sin(time * speed), cy + a * Math.sin(time * speed) * Math.cos(time * speed)]",
        "Figure-8 Lissajous path"});

    m_entries.push_back({"Circular Path", "Spatial",
        "angle = (time / period) * 2 * Math.PI; [cx + radius * Math.cos(angle), cy + radius * Math.sin(angle)]",
        "Complete circular motion path"});

    m_entries.push_back({"Zigzag", "Spatial",
        "seg = Math.floor(time * segments / duration); x = (seg % 2 === 0) ? minX : maxX; x + (time * segments / duration - seg) * (maxX - minX) * (seg % 2 === 0 ? 1 : -1)",
        "Zigzag horizontal motion"});

    // Physics
    m_entries.push_back({"Gravity (Free Fall)", "Physics",
        "0.5 * g * time * time",
        "Free fall under gravity"});

    m_entries.push_back({"Projectile", "Physics",
        "x = v0x * time; y = v0y * time + 0.5 * g * time * time; [x, y]",
        "Projectile motion with gravity"});

    m_entries.push_back({"Elastic Collision", "Physics",
        "v = v0 * Math.cos(freq * time * Math.PI) * Math.exp(-decay * time)",
        "Elastic spring-back motion"});

    m_entries.push_back({"Damped Oscillator", "Physics",
        "A * Math.exp(-gamma * time) * Math.cos(omega * time + phi)",
        "Damped harmonic oscillator"});

    m_entries.push_back({"Simple Pendulum", "Physics",
        "maxAngle * Math.cos(Math.sqrt(g / length) * time)",
        "Simple pendulum angular displacement"});

    m_entries.push_back({"Spring Mass", "Physics",
        "x0 + A * Math.cos(Math.sqrt(k / m) * time) * Math.exp(-b * time / (2 * m))",
        "Damped spring-mass system"});

    // Typography
    m_entries.push_back({"Char Index", "Typography",
        "text.sourceText.charAt(Math.floor(time * charsPerSec))",
        "Reveal text character by character over time"});

    m_entries.push_back({"Type On", "Typography",
        "text.sourceText.substring(0, Math.floor(time * charsPerSec))",
        "Typewriter text reveal effect"});

    m_entries.push_back({"Text Scramble", "Typography",
        "chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%'; original.split('').map((c, i) => i < revealed ? c : chars[Math.floor(random(chars.length))]).join('')",
        "Random character scramble reveal"});

    m_entries.push_back({"Char Rotate", "Typography",
        "text.sourceText.split('').map((c, i) => c + (i === charIndex ? '\\n' : '')).join('')",
        "Highlight specific character"});

    m_entries.push_back({"Word Reveal", "Typography",
        "words = text.sourceText.split(' '); words.slice(0, Math.floor(time * wordsPerSec)).join(' ')",
        "Reveal text word by word"});

    m_entries.push_back({"Text Width Est", "Typography",
        "text.sourceText.length * fontSize * 0.6",
        "Estimate text width based on character count and font size"});
}

const std::vector<ExpressionLibrary::ExpressionEntry>& ExpressionLibrary::getExpressions() const {
    return m_entries;
}

std::vector<ExpressionLibrary::ExpressionEntry> ExpressionLibrary::getExpressionsInCategory(
    const std::string& cat) const {
    std::vector<ExpressionEntry> result;
    std::copy_if(m_entries.begin(), m_entries.end(), std::back_inserter(result),
                 [&cat](const ExpressionEntry& e) { return e.category == cat; });
    return result;
}

std::string ExpressionLibrary::getExpression(const std::string& name) const {
    for (const auto& e : m_entries) {
        if (e.name == name) return e.expression;
    }
    return "";
}

void ExpressionLibrary::addCustomExpression(const std::string& name, const std::string& category,
                                             const std::string& expression, const std::string& desc) {
    for (auto& e : m_entries) {
        if (e.name == name) {
            e.category = category;
            e.expression = expression;
            e.description = desc;
            return;
        }
    }
    m_entries.push_back({name, category, expression, desc});
}

} // namespace FreeEffect
