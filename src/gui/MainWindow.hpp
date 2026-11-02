#pragma once

#include <QMainWindow>

class QLabel;
class QListWidget;
class QMediaPlayer;
class QAudioOutput;
class QSlider;
class QStackedWidget;
class QTreeWidget;
class QVideoWidget;
class QWidget;
class QString;

namespace CreatorForge {

class TimelineWidget;

class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createActions();
    void createCentralWorkspace();
    void createMediaBrowser();
    void createPropertiesPanel();
    void createTimelinePanel();
    void setPreviewMedia(const QString &path);
    void setVideoAudioMuted(bool muted);

    QListWidget *m_mediaList = nullptr;
    QLabel *m_previewLabel = nullptr;
    QLabel *m_timecodeLabel = nullptr;
    QLabel *m_timelineTimeLabel = nullptr;
    QTreeWidget *m_propertiesTree = nullptr;
    TimelineWidget *m_timelineWidget = nullptr;
    QMediaPlayer *m_mediaPlayer = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
    QStackedWidget *m_previewStack = nullptr;
    QSlider *m_positionSlider = nullptr;
    bool m_isSeeking = false;
    QPushButton *m_videoAudioButton = nullptr;
};

} // namespace CreatorForge
