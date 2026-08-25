#include "test_framework.h"
#include "../core/timeline/composition.h"
#include "../core/commands/command_stack.h"
#include "../core/commands/add_layer_command.h"
#include "../core/commands/remove_layer_command.h"
#include "../core/commands/move_layer_command.h"
#include "../core/commands/set_keyframe_command.h"
#include "../core/commands/remove_keyframe_command.h"
#include "../core/commands/set_property_command.h"

using namespace FreeEffect;

TEST(command_stack_basic) {
    CommandStack stack;
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    
    auto cmd = std::make_shared<AddLayerCommand>(&comp, "Layer 1", LayerType::Video);
    stack.execute(cmd);
    
    ASSERT_EQ(comp.getLayerCount(), 1);
    ASSERT_TRUE(stack.canUndo());
    ASSERT_FALSE(stack.canRedo());
}

TEST(command_stack_undo_redo) {
    CommandStack stack;
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    
    auto cmd = std::make_shared<AddLayerCommand>(&comp, "Layer 1", LayerType::Video);
    stack.execute(cmd);
    ASSERT_EQ(comp.getLayerCount(), 1);
    
    stack.undo();
    ASSERT_EQ(comp.getLayerCount(), 0);
    ASSERT_FALSE(stack.canUndo());
    ASSERT_TRUE(stack.canRedo());
    
    stack.redo();
    ASSERT_EQ(comp.getLayerCount(), 1);
    ASSERT_TRUE(stack.canUndo());
    ASSERT_FALSE(stack.canRedo());
}

TEST(command_stack_max_undo) {
    CommandStack stack(2);
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    
    stack.execute(std::make_shared<AddLayerCommand>(&comp, "L1", LayerType::Video));
    stack.execute(std::make_shared<AddLayerCommand>(&comp, "L2", LayerType::Video));
    stack.execute(std::make_shared<AddLayerCommand>(&comp, "L3", LayerType::Video));
    
    ASSERT_EQ(comp.getLayerCount(), 3);
    
    stack.undo();
    ASSERT_EQ(comp.getLayerCount(), 2);
    stack.undo();
    ASSERT_EQ(comp.getLayerCount(), 1);
    // Max undo is 2, so first add is gone
}

TEST(add_layer_command) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    
    auto cmd = std::make_shared<AddLayerCommand>(&comp, "My Layer", LayerType::Video);
    cmd->execute();
    
    ASSERT_EQ(comp.getLayerCount(), 1);
    ASSERT_EQ(comp.getLayer(0)->getName(), "My Layer");
    
    cmd->undo();
    ASSERT_EQ(comp.getLayerCount(), 0);
}

TEST(add_layer_command_at_index) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    
    auto cmd1 = std::make_shared<AddLayerCommand>(&comp, "Layer 1", LayerType::Video);
    cmd1->execute();
    
    auto cmd2 = std::make_shared<AddLayerCommand>(&comp, "Layer 2", LayerType::Audio, 0);
    cmd2->execute();
    
    ASSERT_EQ(comp.getLayerCount(), 2);
    ASSERT_EQ(comp.getLayer(0)->getName(), "Layer 2");
    ASSERT_EQ(comp.getLayer(1)->getName(), "Layer 1");
}

TEST(remove_layer_command) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    auto layer = comp.addLayer("To Remove", LayerType::Video);
    UUID layerId = layer->getId();
    
    auto cmd = std::make_shared<RemoveLayerCommand>(&comp, layerId);
    cmd->execute();
    ASSERT_EQ(comp.getLayerCount(), 0);
    
    cmd->undo();
    ASSERT_EQ(comp.getLayerCount(), 1);
    ASSERT_EQ(comp.getLayer(0)->getName(), "To Remove");
}

TEST(move_layer_command) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    comp.addLayer("Layer 1", LayerType::Video);
    comp.addLayer("Layer 2", LayerType::Audio);
    
    auto cmd = std::make_shared<MoveLayerCommand>(&comp, 1, 0);
    cmd->execute();
    
    ASSERT_EQ(comp.getLayer(0)->getName(), "Layer 2");
    ASSERT_EQ(comp.getLayer(1)->getName(), "Layer 1");
    
    cmd->undo();
    ASSERT_EQ(comp.getLayer(0)->getName(), "Layer 1");
    ASSERT_EQ(comp.getLayer(1)->getName(), "Layer 2");
}

TEST(set_keyframe_command) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    auto layer = comp.addLayer("Layer", LayerType::Video);
    
    auto cmd = std::make_shared<SetKeyframeCommand>(layer.get(), "Opacity", 0.0, 50.0);
    cmd->execute();
    
    ASSERT_TRUE(std::abs(layer->getOpacity().getValueAtTime(0.0) - 50.0) < 1e-6);
    
    cmd->undo();
    ASSERT_TRUE(!layer->getOpacity().hasKeyframes());
}

TEST(remove_keyframe_command) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    auto layer = comp.addLayer("Layer", LayerType::Video);
    layer->getOpacity().addKeyframe(Keyframe(0.0, 75.0));
    
    auto cmd = std::make_shared<RemoveKeyframeCommand>(layer.get(), "Opacity", 0.0);
    cmd->execute();
    ASSERT_TRUE(!layer->getOpacity().hasKeyframes());
    
    cmd->undo();
    ASSERT_TRUE(layer->getOpacity().hasKeyframes());
    ASSERT_TRUE(std::abs(layer->getOpacity().getValueAtTime(0.0) - 75.0) < 1e-6);
}

TEST(set_property_command) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    auto layer = comp.addLayer("Layer", LayerType::Video);
    
    auto cmd = std::make_shared<SetPropertyCommand>(layer.get(), "Scale", 200.0);
    cmd->execute();
    ASSERT_EQ(layer->getScale().getDefaultValue(), 200.0);
    
    cmd->undo();
    ASSERT_EQ(layer->getScale().getDefaultValue(), 100.0);
}

TEST(command_stack_clear) {
    CommandStack stack;
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    
    stack.execute(std::make_shared<AddLayerCommand>(&comp, "L1", LayerType::Video));
    stack.execute(std::make_shared<AddLayerCommand>(&comp, "L2", LayerType::Video));
    
    ASSERT_TRUE(stack.canUndo());
    stack.clear();
    ASSERT_FALSE(stack.canUndo());
    ASSERT_FALSE(stack.canRedo());
}

TEST(command_descriptions) {
    AddLayerCommand addCmd(nullptr, "Test", LayerType::Video);
    ASSERT_TRUE(addCmd.getDescription().find("Add Layer") != std::string::npos);
    
    SetKeyframeCommand kfCmd(nullptr, "Opacity", 0.0, 50.0);
    ASSERT_TRUE(kfCmd.getDescription().find("Set Keyframe") != std::string::npos);
    
    SetPropertyCommand propCmd(nullptr, "Scale", 100.0);
    ASSERT_TRUE(propCmd.getDescription().find("Set Property") != std::string::npos);
}
