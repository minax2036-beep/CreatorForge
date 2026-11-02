#pragma once

#include <QColor>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QWidget>

#include <vector>

namespace CreatorForge {

class TimelineWidget final : public QWidget
{
public:
    enum Track {
        VideoTrack = 0,
        OverlayTrack,
        SubtitleTrack,
        AudioTrack,
        EffectsTrack
    };

    explicit TimelineWidget(QWidget *parent = nullptr);

    int addClip(int track, const QString &name, double startSeconds,
                double durationSeconds, const QColor &color);
    void setSnapEnabled(bool enabled);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    struct Clip {
        int id = 0;
        int track = 0;
        QString name;
        double start = 0.0;
        double duration = 1.0;
        QColor color;
    };

    enum class DragMode { None, Move, ResizeLeft, ResizeRight };

    static constexpr int LabelWidth = 92;
    static constexpr int RulerHeight = 28;
    static constexpr int TrackHeight = 42;
    static constexpr int TrackCount = 5;
    static constexpr double PixelsPerSecond = 40.0;

    QRectF clipRect(const Clip &clip) const;
    int trackAtY(int y) const;
    Clip *clipAt(const QPoint &point, DragMode *mode = nullptr);
    void updateCanvasSize();

    std::vector<Clip> m_clips;
    int m_nextId = 1;
    int m_selectedId = -1;
    int m_draggedId = -1;
    DragMode m_dragMode = DragMode::None;
    QPoint m_pressPosition;
    double m_originalStart = 0.0;
    double m_originalDuration = 1.0;
    bool m_snapEnabled = true;
};

} // namespace CreatorForge
