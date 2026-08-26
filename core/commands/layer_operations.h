#pragma once

#include "command.h"
#include "../timeline/composition.h"
#include "../timeline/layer.h"
#include "../timeline/types.h"
#include "../timeline/track_matte.h"
#include <string>

namespace FreeEffect {

// Flatten all visible layers into a single pre-rendered layer
class FlattenLayersCommand : public Command {
public:
    FlattenLayersCommand(Composition* comp);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    std::vector<LayerPtr> m_savedLayers;
    LayerPtr m_flattenedLayer;
    bool m_executed = false;
};

// Create null object layer
class CreateNullObjectCommand : public Command {
public:
    CreateNullObjectCommand(Composition* comp, double time);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    double m_time;
    LayerPtr m_layer;
};

// Create solid color layer
class CreateSolidLayerCommand : public Command {
public:
    CreateSolidLayerCommand(Composition* comp, const std::string& name, const Color& color,
                            int width, int height);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    std::string m_name;
    Color m_color;
    int m_width, m_height;
    LayerPtr m_layer;
};

// Create adjustment layer
class CreateAdjustmentLayerCommand : public Command {
public:
    CreateAdjustmentLayerCommand(Composition* comp);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    LayerPtr m_layer;
};

// Create camera layer
class CreateCameraLayerCommand : public Command {
public:
    CreateCameraLayerCommand(Composition* comp, const std::string& name);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    std::string m_name;
    LayerPtr m_layer;
};

// Create light layer
class CreateLightLayerCommand : public Command {
public:
    CreateLightLayerCommand(Composition* comp, const std::string& name, int lightType);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    std::string m_name;
    int m_lightType;
    LayerPtr m_layer;
};

// Create empty text layer
class CreateTextLayerCommand : public Command {
public:
    CreateTextLayerCommand(Composition* comp, const std::string& name);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    std::string m_name;
    LayerPtr m_layer;
};

// Create empty shape layer
class CreateShapeLayerCommand : public Command {
public:
    CreateShapeLayerCommand(Composition* comp, const std::string& name);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    std::string m_name;
    LayerPtr m_layer;
};

// Create null at a specific point
class CreateNullAtPointCommand : public Command {
public:
    CreateNullAtPointCommand(Composition* comp, double x, double y, double time);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getLayer() const { return m_layer; }
private:
    Composition* m_comp;
    double m_x, m_y, m_time;
    LayerPtr m_layer;
};

// Duplicate layer with unique name
class DuplicateLayerCommand : public Command {
public:
    DuplicateLayerCommand(Composition* comp, int layerIndex);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
    LayerPtr getDuplicatedLayer() const { return m_newLayer; }
private:
    Composition* m_comp;
    int m_sourceIndex;
    LayerPtr m_newLayer;
    int m_insertIndex;
};

// Rename layer
class RenameLayerCommand : public Command {
public:
    RenameLayerCommand(Composition* comp, const UUID& layerId, const std::string& newName);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    UUID m_layerId;
    std::string m_newName;
    std::string m_oldName;
};

// Set in/out points to current time
class SetInOutPointsCommand : public Command {
public:
    SetInOutPointsCommand(Composition* comp, int layerIndex, double inTime, double outTime);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    double m_inTime, m_outTime;
    double m_oldStartTime, m_oldDuration;
};

// Trim layer to current time
class TrimLayerToCurrentTimeCommand : public Command {
public:
    TrimLayerToCurrentTimeCommand(Composition* comp, int layerIndex, double currentTime, bool trimEnd);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    double m_currentTime;
    bool m_trimEnd;
    double m_oldStartTime, m_oldDuration;
};

// Slide layer forward/backward in time
class SlideLayerCommand : public Command {
public:
    SlideLayerCommand(Composition* comp, int layerIndex, double deltaTime);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    double m_deltaTime;
};

// Ripple delete: remove layer and close gap
class RippleDeleteCommand : public Command {
public:
    RippleDeleteCommand(Composition* comp, int layerIndex);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    LayerPtr m_removedLayer;
    int m_removedIndex;
    double m_removedStartTime;
    double m_removedDuration;
    std::vector<std::pair<int, double>> m_shiftedLayers; // index, old start time
};

// Enable time remap on layer
class EnableTimeRemapCommand : public Command {
public:
    EnableTimeRemapCommand(Composition* comp, int layerIndex, bool enable);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    bool m_enable;
    bool m_wasEnabled;
};

// Add layer markers at current time
class AddLayerMarkerCommand : public Command {
public:
    AddLayerMarkerCommand(Composition* comp, int layerIndex, double time,
                          const std::string& name, const std::string& comment);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    double m_time;
    std::string m_name;
    std::string m_comment;
    int m_markerIndex = -1;
};

// Convert layer to solo
class ConvertToSoloCommand : public Command {
public:
    ConvertToSoloCommand(Composition* comp, int layerIndex, bool solo);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    bool m_solo;
    std::vector<std::pair<int, bool>> m_prevSoloStates;
};

// Hide other layers
class HideOtherLayersCommand : public Command {
public:
    HideOtherLayersCommand(Composition* comp, int layerIndex);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    std::vector<std::pair<int, bool>> m_prevVisibility;
};

// Lock other layers
class LockOtherLayersCommand : public Command {
public:
    LockOtherLayersCommand(Composition* comp, int layerIndex);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    std::vector<std::pair<int, bool>> m_prevLockStates;
};

// Toggle shy state
class ShyLayerCommand : public Command {
public:
    ShyLayerCommand(Composition* comp, int layerIndex);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    bool m_prevShy;
};

// Set layer quality
class SetQualityCommand : public Command {
public:
    SetQualityCommand(Composition* comp, int layerIndex, int quality);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    int m_quality;
    int m_prevQuality;
};

// Set blending mode
class SetBlendingModeCommand : public Command {
public:
    SetBlendingModeCommand(Composition* comp, int layerIndex, BlendMode mode);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    BlendMode m_mode;
    BlendMode m_prevMode;
};

// Set track matte
class SetTrackMatteCommand : public Command {
public:
    SetTrackMatteCommand(Composition* comp, int layerIndex, int matteIndex, TrackMatteMode mode);
    void execute() override;
    void undo() override;
    std::string getDescription() const override;
private:
    Composition* m_comp;
    int m_layerIndex;
    int m_matteIndex;
    TrackMatteMode m_mode;
    TrackMatteReference m_prevMatte;
};

} // namespace FreeEffect
