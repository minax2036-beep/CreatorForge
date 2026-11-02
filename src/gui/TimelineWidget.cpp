#include "TimelineWidget.hpp"

#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>

#include <algorithm>
#include <cmath>

namespace CreatorForge {
namespace {

QString formatTime(int seconds)
{
    const int minutes = seconds / 60;
    const int remainingSeconds = seconds % 60;
    return QString("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(remainingSeconds, 2, 10, QLatin1Char('0'));
}

} // namespace

TimelineWidget::TimelineWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(RulerHeight + TrackCount * TrackHeight + 2);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateCanvasSize();
}


void TimelineWidget::setPlayheadSeconds(double seconds)
{
    m_playheadSeconds = std::max(0.0, seconds);
    const int playheadX = LabelWidth
        + static_cast<int>(m_playheadSeconds * PixelsPerSecond);
    if (playheadX > width() - 20)
        updateCanvasSize();
    update();
}

void TimelineWidget::setSeekCallback(std::function<void(double)> callback)
{
    m_seekCallback = std::move(callback);
}

void TimelineWidget::setSnapEnabled(bool enabled)
{
    m_snapEnabled = enabled;
}

int TimelineWidget::addClip(int track, const QString &name, double startSeconds,
                            double durationSeconds, const QColor &color,
                            const QString &sourcePath,
                            double sourceDurationSeconds,
                            double sourceOffsetSeconds)
{
    if (track < 0 || track >= TrackCount || durationSeconds <= 0.0)
        return -1;

    Clip clip;
    clip.id = m_nextId++;
    clip.track = track;
    clip.name = name;
    clip.sourcePath = sourcePath;
    clip.start = std::max(0.0, startSeconds);
    clip.sourceDuration = std::max(0.0, sourceDurationSeconds);
    clip.sourceOffset = std::max(0.0, sourceOffsetSeconds);
    clip.duration = std::max(0.25, durationSeconds);
    if (clip.sourceDuration > 0.0) {
        clip.sourceOffset = std::min(clip.sourceOffset, clip.sourceDuration);
        clip.duration = std::min(clip.duration,
                                 clip.sourceDuration - clip.sourceOffset);
    }
    clip.duration = std::max(0.25, clip.duration);
    clip.color = color;
    m_clips.push_back(clip);

    updateCanvasSize();
    update();
    return clip.id;
}

QSize TimelineWidget::sizeHint() const
{
    double latestEnd = std::max(60.0, m_playheadSeconds);
    for (const Clip &clip : m_clips)
        latestEnd = std::max(latestEnd, clip.start + clip.duration);

    const int width = LabelWidth
        + static_cast<int>(std::ceil((latestEnd + 5.0) * PixelsPerSecond));
    return QSize(width, RulerHeight + TrackCount * TrackHeight + 2);
}

QSize TimelineWidget::minimumSizeHint() const
{
    return QSize(LabelWidth + 60 * static_cast<int>(PixelsPerSecond),
                 RulerHeight + TrackCount * TrackHeight + 2);
}

QRectF TimelineWidget::clipRect(const Clip &clip) const
{
    const qreal x = LabelWidth + clip.start * PixelsPerSecond;
    const qreal y = RulerHeight + clip.track * TrackHeight + 6;
    const qreal width = std::max(12.0, clip.duration * PixelsPerSecond);
    return QRectF(x, y, width, TrackHeight - 12);
}

int TimelineWidget::trackAtY(int y) const
{
    if (y < RulerHeight)
        return -1;

    const int track = (y - RulerHeight) / TrackHeight;
    return (track >= 0 && track < TrackCount) ? track : -1;
}

TimelineWidget::Clip *TimelineWidget::clipAt(const QPoint &point, DragMode *mode)
{
    for (auto it = m_clips.rbegin(); it != m_clips.rend(); ++it) {
        const QRectF rect = clipRect(*it);
        if (!rect.contains(point))
            continue;

        if (mode) {
            const qreal edgeDistance = 7.0;
            if (point.x() <= rect.left() + edgeDistance)
                *mode = DragMode::ResizeLeft;
            else if (point.x() >= rect.right() - edgeDistance)
                *mode = DragMode::ResizeRight;
            else
                *mode = DragMode::Move;
        }
        return &(*it);
    }
    return nullptr;
}

void TimelineWidget::updateCanvasSize()
{
    setMinimumSize(sizeHint());
    resize(sizeHint());
    updateGeometry();
}

void TimelineWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#171b22"));

    painter.fillRect(QRect(0, 0, width(), RulerHeight), QColor("#202630"));
    painter.fillRect(QRect(0, RulerHeight, LabelWidth, height()), QColor("#1d222a"));

    painter.setPen(QColor("#aeb8c7"));
    painter.drawText(QRect(8, 0, LabelWidth - 16, RulerHeight),
                     Qt::AlignVCenter | Qt::AlignLeft, "TRACK / TIME");

    const int secondsVisible = static_cast<int>(
        std::ceil((width() - LabelWidth) / PixelsPerSecond));
    for (int second = 0; second <= secondsVisible; second += 5) {
        const int x = LabelWidth + static_cast<int>(second * PixelsPerSecond);
        painter.setPen(QPen(QColor("#343c49"), 1));
        painter.drawLine(x, RulerHeight, x, height());
        painter.setPen(QColor("#aeb8c7"));
        painter.drawText(QRect(x + 4, 0, 60, RulerHeight),
                         Qt::AlignVCenter | Qt::AlignLeft, formatTime(second));
    }

    const QStringList trackNames = {
        "V1  Video", "V2  Overlay", "T1  Subtitle", "A1  Audio", "A2  Effects"
    };

    for (int track = 0; track < TrackCount; ++track) {
        const int y = RulerHeight + track * TrackHeight;
        painter.fillRect(QRect(LabelWidth, y, width() - LabelWidth, TrackHeight),
                         track % 2 == 0 ? QColor("#1b2028") : QColor("#1e242d"));
        painter.setPen(QPen(QColor("#303744"), 1));
        painter.drawLine(0, y + TrackHeight, width(), y + TrackHeight);
        painter.setPen(QColor("#aeb8c7"));
        painter.drawText(QRect(9, y, LabelWidth - 16, TrackHeight),
                         Qt::AlignVCenter | Qt::AlignLeft, trackNames.at(track));
    }

    for (const Clip &clip : m_clips) {
        const QRectF rect = clipRect(clip);
        const bool selected = clip.id == m_selectedId;

        painter.setPen(QPen(selected ? QColor("#e5edf8") : clip.color.darker(125),
                            selected ? 2 : 1));
        painter.setBrush(clip.color);
        painter.drawRoundedRect(rect, 4, 4);

        painter.setPen(Qt::white);
        const QRectF textRect = rect.adjusted(8, 0, -8, 0);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                         painter.fontMetrics().elidedText(
                             clip.name, Qt::ElideRight,
                             std::max(0, static_cast<int>(textRect.width()))));

        if (selected) {
            painter.setPen(QPen(QColor("#f1f5fb"), 2));
            painter.drawLine(rect.left() + 3, rect.top() + 4,
                             rect.left() + 3, rect.bottom() - 4);
            painter.drawLine(rect.right() - 3, rect.top() + 4,
                             rect.right() - 3, rect.bottom() - 4);
        }
    }

    const int playheadX = LabelWidth
        + static_cast<int>(m_playheadSeconds * PixelsPerSecond);
    painter.setPen(QPen(QColor("#ff5b62"), 2));
    painter.drawLine(playheadX, 0, playheadX, height());
    painter.setBrush(QColor("#ff5b62"));
    painter.setPen(Qt::NoPen);
    QPolygon marker;
    marker << QPoint(playheadX - 6, 0) << QPoint(playheadX + 6, 0)
           << QPoint(playheadX, 9);
    painter.drawPolygon(marker);
}

void TimelineWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    DragMode mode = DragMode::None;
    const QPoint clickPosition = event->position().toPoint();
    Clip *clip = clipAt(clickPosition, &mode);
    if (!clip) {
        m_selectedId = -1;
        m_draggedId = -1;
        m_dragMode = DragMode::None;

        if (clickPosition.x() >= LabelWidth && m_seekCallback) {
            const double seconds = std::max(
                0.0, (clickPosition.x() - LabelWidth) / PixelsPerSecond);
            m_seekCallback(seconds);
        }
        update();
        return;
    }

    m_selectedId = clip->id;
    m_draggedId = clip->id;
    m_dragMode = mode;
    m_pressPosition = event->position().toPoint();
    m_originalStart = clip->start;
    m_originalDuration = clip->duration;
    m_originalSourceOffset = clip->sourceOffset;
    setCursor(mode == DragMode::Move ? Qt::SizeAllCursor : Qt::SizeHorCursor);
    update();
}

void TimelineWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggedId >= 0 && (event->buttons() & Qt::LeftButton)) {
        auto it = std::find_if(m_clips.begin(), m_clips.end(),
            [this](const Clip &clip) { return clip.id == m_draggedId; });
        if (it == m_clips.end())
            return;

        const double delta = (event->position().x() - m_pressPosition.x())
                           / PixelsPerSecond;

        const auto snap = [this](double value) {
            return m_snapEnabled ? std::round(value * 2.0) / 2.0 : value;
        };

        if (m_dragMode == DragMode::Move) {
            it->start = std::max(0.0, snap(m_originalStart + delta));
        } else if (m_dragMode == DragMode::ResizeRight) {
            const double end = snap(m_originalStart + m_originalDuration + delta);
            double newDuration = std::max(0.25, end - it->start);
            if (it->sourceDuration > 0.0)
                newDuration = std::min(newDuration,
                    it->sourceDuration - it->sourceOffset);
            it->duration = std::max(0.25, newDuration);
        } else if (m_dragMode == DragMode::ResizeLeft) {
            const double newStart = std::max(0.0,
                std::min(snap(m_originalStart + delta),
                         m_originalStart + m_originalDuration - 0.25));
            const double startDelta = newStart - m_originalStart;
            it->start = newStart;
            it->sourceOffset = std::max(0.0,
                m_originalSourceOffset + startDelta);
            it->duration = m_originalDuration - startDelta;
            if (it->sourceDuration > 0.0)
                it->duration = std::min(it->duration,
                    it->sourceDuration - it->sourceOffset);
            it->duration = std::max(0.25, it->duration);
        }

        updateCanvasSize();
        update();
        return;
    }

    DragMode mode = DragMode::None;
    if (clipAt(event->position().toPoint(), &mode))
        setCursor(mode == DragMode::Move ? Qt::SizeAllCursor : Qt::SizeHorCursor);
    else
        unsetCursor();
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_draggedId = -1;
    m_dragMode = DragMode::None;
    unsetCursor();
}

void TimelineWidget::contextMenuEvent(QContextMenuEvent *event)
{
    DragMode mode = DragMode::None;
    Clip *clip = clipAt(event->pos(), &mode);
    if (!clip) {
        QWidget::contextMenuEvent(event);
        return;
    }

    m_selectedId = clip->id;
    const int selectedId = clip->id;
    QMenu menu(this);
    QAction *removeAction = menu.addAction("Delete Clip");
    if (menu.exec(event->globalPos()) == removeAction) {
        m_clips.erase(std::remove_if(m_clips.begin(), m_clips.end(),
            [selectedId](const Clip &item) { return item.id == selectedId; }),
            m_clips.end());
        m_selectedId = -1;
        updateCanvasSize();
        update();
    } else {
        update();
    }
}

} // namespace CreatorForge
