#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include "../../core/timeline/composition.h"

namespace FreeEffect {

class TimelineCanvas : public QWidget {
    Q_OBJECT
public:
    explicit TimelineCanvas(QWidget* parent = nullptr);

    void setComposition(std::shared_ptr<Composition> comp);
    void setCurrentTime(double time);
    void setPixelsPerSecond(double pps);
    void setSelectedLayer(int index);
    double getPixelsPerSecond() const { return m_pixelsPerSecond; }

signals:
    void timeClicked(double time);
    void layerClicked(int index);
    void zoomChanged(double pixelsPerSecond);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawTimeRuler(QPainter& painter, const QRect& rulerRect);
    void drawLayerBars(QPainter& painter, const QRect& contentRect);
    void drawPlayhead(QPainter& painter, const QRect& fullRect);
    void drawWorkArea(QPainter& painter, const QRect& rulerRect);
    void drawAlternatingRows(QPainter& painter, const QRect& contentRect);
    void drawKeyframes(QPainter& painter, const QRect& contentRect);

    double xToTime(int x) const;
    int timeToX(double time) const;

    static constexpr int RULER_HEIGHT = 28;
    static constexpr int LAYER_ROW_HEIGHT = 24;
    static constexpr int LEFT_MARGIN = 0;

    std::shared_ptr<Composition> m_composition;
    double m_currentTime = 0.0;
    double m_pixelsPerSecond = 60.0;
    int m_selectedLayer = -1;
    int m_scrollX = 0;
    double m_workAreaStart = 0.0;
    double m_workAreaEnd = 10.0;
};

} // namespace FreeEffect
