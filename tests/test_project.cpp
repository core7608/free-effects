#include "test_framework.h"
#include "../core/project/asset_reference.h"
#include "../core/project/project_state.h"
#include "../core/project/missing_footage_handler.h"
#include <filesystem>
#include <fstream>

using namespace FreeEffect;

TEST(asset_reference_creation) {
    AssetReference asset("/path/to/file.mp4", "file.mp4", AssetType::Video);
    
    ASSERT_TRUE(!asset.getId().empty());
    ASSERT_EQ(asset.getName(), "file.mp4");
    ASSERT_EQ(asset.getType(), AssetType::Video);
}

TEST(asset_reference_missing_file) {
    AssetReference asset("/nonexistent/path.mp4", "missing.mp4", AssetType::Video);
    ASSERT_EQ(asset.getStatus(), AssetStatus::Missing);
}

TEST(asset_reference_existing_file) {
    auto tempDir = std::filesystem::temp_directory_path();
    std::string tempPath = (tempDir / "freeeffect_test_asset.txt").string();
    std::ofstream f(tempPath);
    f << "test";
    f.close();
    
    AssetReference asset(tempPath, "test.txt", AssetType::Image);
    ASSERT_EQ(asset.getStatus(), AssetStatus::Available);
    
    std::filesystem::remove(tempPath);
}

TEST(project_state_assets) {
    ProjectState project;
    
    auto asset = project.addAsset("/path/to/video.mp4", "video.mp4", AssetType::Video);
    ASSERT_EQ(project.getAssets().size(), 1u);
    ASSERT_TRUE(project.getAssetById(asset->getId()) != nullptr);
    
    project.removeAsset(asset->getId());
    ASSERT_EQ(project.getAssets().size(), 0u);
}

TEST(project_state_compositions) {
    ProjectState project;
    
    auto comp = project.addComposition("Comp 1", {1920, 1080}, {30.0}, 10.0);
    ASSERT_EQ(project.getCompositions().size(), 1u);
    ASSERT_TRUE(project.getCompositionById(comp->getId()) != nullptr);
    
    project.removeComposition(comp->getId());
    ASSERT_EQ(project.getCompositions().size(), 0u);
}

TEST(project_state_modified) {
    ProjectState project;
    ASSERT_FALSE(project.isModified());
    
    project.addAsset("/path.mp4", "test.mp4", AssetType::Video);
    ASSERT_TRUE(project.isModified());
    
    project.setModified(false);
    ASSERT_FALSE(project.isModified());
}

TEST(project_state_clear) {
    ProjectState project;
    project.addAsset("/path.mp4", "test.mp4", AssetType::Video);
    project.addComposition("Comp 1", {1920, 1080}, {30.0}, 10.0);
    project.setFilePath("/test/project.feproj");
    
    project.clear();
    ASSERT_EQ(project.getAssets().size(), 0u);
    ASSERT_EQ(project.getCompositions().size(), 0u);
    ASSERT_TRUE(project.getFilePath().empty());
}

TEST(missing_footage_none) {
    ProjectState project;
    MissingFootageHandler handler(&project);
    
    // No assets = no missing footage
    ASSERT_FALSE(handler.hasMissingFootage());
    ASSERT_TRUE(handler.checkForMissingFootage().empty());
}

TEST(missing_footage_detection) {
    ProjectState project;
    project.addAsset("/nonexistent/video.mp4", "video.mp4", AssetType::Video);
    
    MissingFootageHandler handler(&project);
    ASSERT_TRUE(handler.hasMissingFootage());
    
    auto missing = handler.checkForMissingFootage();
    ASSERT_EQ(missing.size(), 1u);
    ASSERT_EQ(missing[0].name, "video.mp4");
}

TEST(missing_footage_relink) {
    ProjectState project;
    auto asset = project.addAsset("/old/path/video.mp4", "video.mp4", AssetType::Video);
    
    MissingFootageHandler handler(&project);
    
    auto tempDir = std::filesystem::temp_directory_path();
    std::string newPath = (tempDir / "freeeffect_relink_test.mp4").string();
    std::ofstream f(newPath);
    f << "test";
    f.close();
    
    bool result = handler.relinkAsset(asset->getId(), newPath);
    ASSERT_TRUE(result);
    ASSERT_EQ(asset->getPath(), newPath);
    
    std::filesystem::remove(newPath);
}

TEST(project_state_settings) {
    ProjectState project;
    
    ASSERT_TRUE(std::abs(project.getSettings().frameRateBase - 30.0) < 1e-6);
    project.getSettings().frameRateBase = 24.0;
    ASSERT_TRUE(std::abs(project.getSettings().frameRateBase - 24.0) < 1e-6);
}
