#pragma once
#include <string>
#include <map>
#include <variant>

namespace FreeEffect {

using PrefValue = std::variant<bool, int, double, std::string>;

class Preferences {
public:
    static Preferences& instance();
    
    void set(const std::string& key, PrefValue value);
    PrefValue get(const std::string& key, PrefValue defaultValue = {}) const;
    bool getBool(const std::string& key, bool def = false) const;
    int getInt(const std::string& key, int def = 0) const;
    double getDouble(const std::string& key, double def = 0.0) const;
    std::string getString(const std::string& key, const std::string& def = "") const;
    
    void save();
    void load();
    
    // AE-style preference categories
    static constexpr const char* kGeneral = "general.";
    static constexpr const char* kDisplay = "display.";
    static constexpr const char* kImport = "import.";
    static constexpr const char* kOutput = "output.";
    static constexpr const char* kGrid = "grid.";
    static constexpr const char* kLabels = "labels.";
    static constexpr const char* kMemory = "memory.";
    static constexpr const char* kVideoPreview = "videoPreview.";
    static constexpr const char* kPreviews = "previews.";
    static constexpr const char* kScripting = "scripting.";
    static constexpr const char* kAudio = "audio.";
    static constexpr const char* kAutoSave = "autoSave.";
    static constexpr const char* kAppearance = "appearance.";

    // General preferences
    void setDefaultDuration(double seconds) { set(kGeneral + std::string("defaultDuration"), seconds); }
    double getDefaultDuration() const { return getDouble(kGeneral + std::string("defaultDuration"), 10.0); }
    void setDefaultFrameRate(double fps) { set(kGeneral + std::string("defaultFrameRate"), fps); }
    double getDefaultFrameRate() const { return getDouble(kGeneral + std::string("defaultFrameRate"), 30.0); }
    void setUndoLevels(int levels) { set(kGeneral + std::string("undoLevels"), levels); }
    int getUndoLevels() const { return getInt(kGeneral + std::string("undoLevels"), 32); }
    void setDefaultProjectFolder(const std::string& path) { set(kGeneral + std::string("defaultProjectFolder"), path); }
    std::string getDefaultProjectFolder() const { return getString(kGeneral + std::string("defaultProjectFolder"), ""); }

    // Preview preferences
    void setPreviewResolution(int resolution) { set(kPreviews + std::string("resolution"), resolution); }
    int getPreviewResolution() const { return getInt(kPreviews + std::string("resolution"), 1); }
    void setDefaultRAMPreviewDuration(double seconds) { set(kPreviews + std::string("ramDuration"), seconds); }
    double getDefaultRAMPreviewDuration() const { return getDouble(kPreviews + std::string("ramDuration"), 8.0); }

    // Display preferences
    void setMotionPathStyle(int style) { set(kDisplay + std::string("motionPathStyle"), style); }
    int getMotionPathStyle() const { return getInt(kDisplay + std::string("motionPathStyle"), 0); }
    void setMotionPathKeyframeStyle(int style) { set(kDisplay + std::string("motionPathKeyframeStyle"), style); }
    int getMotionPathKeyframeStyle() const { return getInt(kDisplay + std::string("motionPathKeyframeStyle"), 0); }
    void setDefaultGridSize(int pixels) { set(kDisplay + std::string("defaultGridSize"), pixels); }
    int getDefaultGridSize() const { return getInt(kDisplay + std::string("defaultGridSize"), 50); }

    // Import preferences
    void setDefaultImportAlpha(int mode) { set(kImport + std::string("alphaMode"), mode); }
    int getDefaultImportAlpha() const { return getInt(kImport + std::string("alphaMode"), 0); }
    void setDefaultImportFrameRate(double fps) { set(kImport + std::string("frameRate"), fps); }
    double getDefaultImportFrameRate() const { return getDouble(kImport + std::string("frameRate"), 30.0); }
    void setSequenceFootageMissing(int action) { set(kImport + std::string("sequenceMissing"), action); }
    int getSequenceFootageMissing() const { return getInt(kImport + std::string("sequenceMissing"), 0); }

    // Output preferences
    void setDefaultOutputFolder(const std::string& path) { set(kOutput + std::string("defaultFolder"), path); }
    std::string getDefaultOutputFolder() const { return getString(kOutput + std::string("defaultFolder"), ""); }
    void setDefaultTemplate(const std::string& name) { set(kOutput + std::string("defaultTemplate"), name); }
    std::string getDefaultTemplate() const { return getString(kOutput + std::string("defaultTemplate"), ""); }

    // Memory & Performance preferences
    void setRAMReserved(int mb) { set(kMemory + std::string("reservedMB"), mb); }
    int getRAMReserved() const { return getInt(kMemory + std::string("reservedMB"), 0); }
    void setConcurrentFrames(int count) { set(kMemory + std::string("concurrentFrames"), count); }
    int getConcurrentFrames() const { return getInt(kMemory + std::string("concurrentFrames"), 4); }
    void setDiskCachePath(const std::string& path) { set(kMemory + std::string("diskCachePath"), path); }
    std::string getDiskCachePath() const { return getString(kMemory + std::string("diskCachePath"), ""); }
    void setDiskCacheSizeMB(size_t mb) { set(kMemory + std::string("diskCacheSizeMB"), static_cast<int>(mb)); }
    int getDiskCacheSizeMB() const { return getInt(kMemory + std::string("diskCacheSizeMB"), 1024); }

    // Scripting & Expressions preferences
    void setEnableScripting(bool enabled) { set(kScripting + std::string("enabled"), enabled); }
    bool getEnableScripting() const { return getBool(kScripting + std::string("enabled"), true); }
    void setExpressionEngine(const std::string& engine) { set(kScripting + std::string("expressionEngine"), engine); }
    std::string getExpressionEngine() const { return getString(kScripting + std::string("expressionEngine"), "javascript"); }
    void setAllowScriptsToWrite(bool enabled) { set(kScripting + std::string("allowWrite"), enabled); }
    bool getAllowScriptsToWrite() const { return getBool(kScripting + std::string("allowWrite"), false); }

    // Audio preferences
    void setAudioHardwareSampleRate(int rate) { set(kAudio + std::string("sampleRate"), rate); }
    int getAudioHardwareSampleRate() const { return getInt(kAudio + std::string("sampleRate"), 48000); }
    void setAudioHardwareBufferSize(int samples) { set(kAudio + std::string("bufferSize"), samples); }
    int getAudioHardwareBufferSize() const { return getInt(kAudio + std::string("bufferSize"), 2048); }
    void setDefaultAudioDuration(double seconds) { set(kAudio + std::string("defaultDuration"), seconds); }
    double getDefaultAudioDuration() const { return getDouble(kAudio + std::string("defaultDuration"), 30.0); }

    // Auto-Save preferences
    void setAutoSaveEnabled(bool enabled) { set(kAutoSave + std::string("enabled"), enabled); }
    bool getAutoSaveEnabled() const { return getBool(kAutoSave + std::string("enabled"), true); }
    void setAutoSaveInterval(int minutes) { set(kAutoSave + std::string("interval"), minutes); }
    int getAutoSaveInterval() const { return getInt(kAutoSave + std::string("interval"), 20); }
    void setAutoSaveMaxSaves(int max) { set(kAutoSave + std::string("maxSaves"), max); }
    int getAutoSaveMaxSaves() const { return getInt(kAutoSave + std::string("maxSaves"), 5); }

    // Appearance preferences
    void setUIColor(const std::string& name, const std::string& color) { set(kAppearance + name, color); }
    std::string getUIColor(const std::string& name, const std::string& def = "#333333") const {
        return getString(kAppearance + name, def);
    }
    void setUIBrightness(double brightness) { set(kAppearance + std::string("brightness"), brightness); }
    double getUIBrightness() const { return getDouble(kAppearance + std::string("brightness"), 0.5); }

private:
    Preferences() { load(); }
    std::map<std::string, PrefValue> m_values;
    std::string getConfigPath() const;
};

} // namespace FreeEffect
