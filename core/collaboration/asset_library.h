#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cstdint>

namespace FreeEffect {

enum class LibraryAssetType { Color, Graphic, CharacterStyle, LayerStyle, Animation, Comp };

struct LibraryAsset {
    std::string id;
    std::string name;
    LibraryAssetType type;
    std::string filePath;
    std::vector<uint8_t> thumbnailData;
    int thumbnailWidth = 0;
    int thumbnailHeight = 0;
    std::unordered_map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point createdAt;
};

class AssetLibrary {
public:
    bool createLibrary(const std::string& name, const std::string& path);
    bool openLibrary(const std::string& path);
    void closeLibrary();
    bool isOpen() const { return m_open; }

    bool addAsset(const LibraryAsset& asset);
    bool removeAsset(const std::string& assetId);
    bool updateAsset(const std::string& assetId, const LibraryAsset& asset);

    std::vector<LibraryAsset> getAssets() const { return m_assets; }
    std::vector<LibraryAsset> getAssetsByType(LibraryAssetType type) const;
    LibraryAsset getAsset(const std::string& assetId) const;

    // Color swatches
    bool addColorSwatch(const std::string& name, double r, double g, double b, double a = 1.0);
    bool updateColorSwatch(const std::string& name, double r, double g, double b, double a = 1.0);

    // Character styles
    bool addCharacterStyle(const std::string& name, const std::string& fontFamily,
                           double size, double leading, double tracking);

    // Share library
    bool exportLibrary(const std::string& path);
    bool importLibrary(const std::string& path);

    const std::string& getName() const { return m_name; }
    const std::string& getPath() const { return m_path; }

private:
    std::string m_name;
    std::string m_path;
    bool m_open = false;
    std::vector<LibraryAsset> m_assets;

    std::string generateId() const;
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);
};

} // namespace FreeEffect
