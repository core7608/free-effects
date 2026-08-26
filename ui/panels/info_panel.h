#pragma once
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QString>

namespace FreeEffect {

class InfoPanel : public QWidget {
    Q_OBJECT
public:
    explicit InfoPanel(QWidget* parent = nullptr);
    
    void updateInfo(double time, int frame, int compWidth, int compHeight, double fps);
    void updateMousePosition(int x, int y, const QString& colorInfo);
    void updateLayerInfo(const QString& layerName, const QString& layerType);
    
signals:
    void positionChanged(int x, int y);

private:
    QLabel* m_timeLabel;
    QLabel* m_frameLabel;
    QLabel* m_resolutionLabel;
    QLabel* m_fpsLabel;
    QLabel* m_mousePosLabel;
    QLabel* m_colorLabel;
    QLabel* m_layerLabel;
};

} // namespace FreeEffect
