#include "test_framework.h"
#include "../core/io/importer.h"
#include "../core/io/project_file.h"
#include "../core/project/project_state.h"
#include <filesystem>
#include <fstream>

using namespace FreeEffect;

TEST(importer_detect_video) {
    Importer importer(nullptr);
    ASSERT_TRUE(importer.isSupportedFormat("test.mp4"));
    ASSERT_TRUE(importer.isSupportedFormat("test.mov"));
    ASSERT_TRUE(importer.isSupportedFormat("test.avi"));
}

TEST(importer_detect_image) {
    Importer importer(nullptr);
    ASSERT_TRUE(importer.isSupportedFormat("test.png"));
    ASSERT_TRUE(importer.isSupportedFormat("test.jpg"));
    ASSERT_TRUE(importer.isSupportedFormat("test.jpeg"));
}

TEST(importer_detect_audio) {
    Importer importer(nullptr);
    ASSERT_TRUE(importer.isSupportedFormat("test.wav"));
    ASSERT_TRUE(importer.isSupportedFormat("test.mp3"));
    ASSERT_TRUE(importer.isSupportedFormat("test.ogg"));
}

TEST(importer_unsupported_format) {
    Importer importer(nullptr);
    ASSERT_FALSE(importer.isSupportedFormat("test.txt"));
    ASSERT_FALSE(importer.isSupportedFormat("test.exe"));
}

TEST(importer_file_type_detection) {
    Importer importer(nullptr);
    ASSERT_EQ(importer.detectFileType("test.mp4"), AssetType::Video);
    ASSERT_EQ(importer.detectFileType("test.png"), AssetType::Image);
    ASSERT_EQ(importer.detectFileType("test.wav"), AssetType::Audio);
}

TEST(importer_nonexistent_file) {
    ProjectState project;
    Importer importer(&project);
    
    auto asset = importer.importFile("/nonexistent/file.mp4");
    ASSERT_TRUE(asset == nullptr);
}

TEST(importer_existing_file) {
    ProjectState project;
    Importer importer(&project);
    
    // Create temp file
    std::string tempPath = "/tmp/freeeffect_import_test.png";
    std::ofstream f(tempPath);
    f << "fake image data";
    f.close();
    
    auto asset = importer.importFile(tempPath);
    ASSERT_TRUE(asset != nullptr);
    ASSERT_EQ(asset->getName(), "freeeffect_import_test.png");
    ASSERT_EQ(project.getAssets().size(), 1u);
    
    std::filesystem::remove(tempPath);
}

TEST(project_file_save) {
    ProjectState project;
    project.addComposition("Comp 1", {1920, 1080}, {30.0}, 10.0);
    
    std::string savePath = "/tmp/freeeffect_test_project.feproj";
    
    ProjectFile pf;
    bool result = pf.save(project, savePath);
    ASSERT_TRUE(result);
    ASSERT_TRUE(std::filesystem::exists(savePath));
    
    std::filesystem::remove(savePath);
}

TEST(project_file_save_load_roundtrip) {
    ProjectState project;
    project.addComposition("My Comp", {1920, 1080}, {30.0}, 5.0);
    
    std::string path = "/tmp/freeeffect_roundtrip.feproj";
    
    ProjectFile pf;
    pf.save(project, path);
    
    ProjectState loaded;
    auto result = pf.load(path, loaded);
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.errorMessage.empty());
    
    std::filesystem::remove(path);
}

TEST(project_file_invalid_format) {
    ProjectState project;
    
    std::string path = "/tmp/freeeffect_invalid.feproj";
    std::ofstream f(path);
    f << "this is not a valid project file";
    f.close();
    
    ProjectFile pf;
    auto result = pf.load(path, project);
    ASSERT_FALSE(result.success);
    ASSERT_TRUE(!result.errorMessage.empty());
    
    std::filesystem::remove(path);
}

TEST(project_file_nonexistent) {
    ProjectState project;
    
    ProjectFile pf;
    auto result = pf.load("/nonexistent/path.feproj", project);
    ASSERT_FALSE(result.success);
}
