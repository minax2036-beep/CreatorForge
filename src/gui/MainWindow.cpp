#include "MainWindow.hpp"
#include "TimelineWidget.hpp"

#include <QAction>
#include <QApplication>
#include <QAudioOutput>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QPolygon>
#include <QProcess>
#include <QScrollArea>
#include <QSlider>
#include <QStackedWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QUrl>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QColor>

namespace CreatorForge {
namespace {

QLabel *makeSectionLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName("sectionLabel");
    return label;
}

QString formatTimecode(qint64 milliseconds)
{
    const qint64 totalSeconds = qMax<qint64>(0, milliseconds / 1000);
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds / 60) % 60;
    const qint64 seconds = totalSeconds % 60;
    const qint64 millis = qMax<qint64>(0, milliseconds % 1000);
    return QString("%1:%2:%3.%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(millis, 3, 10, QChar('0'));
}

double probeMediaDuration(const QString &path)
{
    QProcess process;
    process.start("ffprobe", {
        "-v", "error",
        "-show_entries", "format=duration",
        "-of", "default=noprint_wrappers=1:nokey=1",
        path
    });

    if (!process.waitForStarted(1000) || !process.waitForFinished(3000))
        return 0.0;

    bool ok = false;
    const double duration = QString::fromUtf8(process.readAllStandardOutput())
                                .trimmed().toDouble(&ok);
    return ok && duration > 0.0 ? duration : 0.0;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("CreatorForge");
    resize(1500, 940);
    setMinimumSize(1050, 680);

    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #17191e;
            color: #e4e7ed;
            font-family: "Inter", "Noto Sans", sans-serif;
            font-size: 10pt;
        }
        QMenuBar {
            background: #17191e;
            color: #cbd0da;
            border-bottom: 1px solid #2a2e37;
            padding: 4px 8px;
        }
        QMenuBar::item { padding: 6px 10px; background: transparent; }
        QMenuBar::item:selected { background: #292e38; border-radius: 4px; }
        QMenu {
            background: #22262e;
            border: 1px solid #383e49;
            padding: 5px;
        }
        QMenu::item { padding: 7px 28px 7px 12px; }
        QMenu::item:selected { background: #343c4b; border-radius: 3px; }
        QToolBar {
            background: #1c1f25;
            border: none;
            border-bottom: 1px solid #2c313b;
            spacing: 6px;
            padding: 7px 10px;
        }
        QToolButton, QPushButton {
            background: #292e38;
            color: #dce1ea;
            border: 1px solid #3a414e;
            border-radius: 5px;
            padding: 7px 12px;
        }
        QToolButton:hover, QPushButton:hover { background: #353d4b; }
        QToolButton:checked, QPushButton:checked {
            background: #294c78;
            border-color: #4b83c4;
            color: white;
        }
        QDockWidget {
            color: #dce1ea;
            font-weight: 600;
            titlebar-close-icon: none;
            titlebar-normal-icon: none;
        }
        QDockWidget::title {
            background: #20242b;
            border-bottom: 1px solid #303641;
            padding: 9px 10px;
        }
        QFrame#panelSurface {
            background: #1e2229;
            border: 1px solid #303641;
            border-radius: 6px;
        }
        QLabel#sectionLabel {
            color: #aeb7c6;
            font-size: 9pt;
            font-weight: 700;
            letter-spacing: 1px;
        }
        QLabel#mutedLabel { color: #818b9b; }
        QLabel#previewCanvas {
            background: #101216;
            border: 1px solid #343a45;
            border-radius: 5px;
            color: #737d8d;
        }
        QListWidget, QTreeWidget {
            background: #1b1f26;
            alternate-background-color: #20252d;
            border: 1px solid #303641;
            border-radius: 4px;
            outline: none;
            padding: 4px;
        }
        QListWidget::item, QTreeWidget::item { padding: 6px; }
        QListWidget::item:selected, QTreeWidget::item:selected {
            background: #294c78;
            border-radius: 3px;
        }
        QHeaderView::section {
            background: #252a33;
            color: #aeb7c6;
            border: none;
            border-bottom: 1px solid #363d49;
            padding: 6px;
        }
        QStatusBar {
            background: #1c1f25;
            color: #8e98a8;
            border-top: 1px solid #2c313b;
        }
        QSplitter::handle { background: #2b3039; }
        QSplitter::handle:horizontal { width: 3px; }
        QSplitter::handle:vertical { height: 3px; }
        QScrollBar:vertical {
            background: #1b1f26; width: 10px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #414957; min-height: 24px; border-radius: 4px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");

    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    createActions();
    createCentralWorkspace();
    createMediaBrowser();
    createPropertiesPanel();
    createTimelinePanel();

    statusBar()->showMessage("Ready  •  CreatorForge 0.2.0 — Qt Multimedia");
}

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *newProjectAction = fileMenu->addAction("New Project");
    QAction *openProjectAction = fileMenu->addAction("Open Project...");
    fileMenu->addSeparator();
    QAction *importMediaAction = fileMenu->addAction("Import Media...");
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("Exit");

    connect(newProjectAction, &QAction::triggered, this, [this] {
        QMessageBox::information(this, "New Project",
            "Project creation will be implemented in a future milestone.");
    });

    connect(openProjectAction, &QAction::triggered, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, "Open CreatorForge Project", {}, "CreatorForge Project (*.cforge);;All Files (*)");
        if (!path.isEmpty()) {
            statusBar()->showMessage(
                "Project loading is not implemented yet: " + QFileInfo(path).fileName(), 5000);
        }
    });

    connect(importMediaAction, &QAction::triggered, this, [this] {
        const QStringList paths = QFileDialog::getOpenFileNames(
            this, "Import Media", {},
            "Media Files (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.mp4 *.mkv *.mov *.mp3 *.wav *.flac);;All Files (*)");
        for (const QString &path : paths) {
            auto *item = new QListWidgetItem(QFileInfo(path).fileName(), m_mediaList);
            item->setData(Qt::UserRole, path);
        }
        if (!paths.isEmpty()) {
            statusBar()->showMessage(QString("Imported %1 item(s) into the session").arg(paths.size()), 5000);
        }
    });

    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("Undo")->setEnabled(false);
    editMenu->addAction("Redo")->setEnabled(false);

    QMenu *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("Reset Workspace", this, [this] {
        statusBar()->showMessage("Workspace reset is not implemented yet.", 3000);
    });

    QMenu *projectMenu = menuBar()->addMenu("&Project");
    projectMenu->addAction("Project Settings", this, [this] {
        QMessageBox::information(this, "Project Settings",
            "Project settings will be added with the project manager.");
    });

    QMenu *exportMenu = menuBar()->addMenu("&Export");
    exportMenu->addAction("Export...", this, [this] {
        QMessageBox::information(this, "Export",
            "Export engines will be integrated in a later milestone.");
    });

    menuBar()->addMenu("&Help")->addAction("About CreatorForge", this, [this] {
        QMessageBox::about(this, "About CreatorForge",
            "<h2>CreatorForge</h2><p>A unified creative suite for video editing, animation, "
            "image compositing and media processing.</p><p>Early development foundation.</p>");
    });

    auto *toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->addAction(newProjectAction);
    toolbar->addAction(openProjectAction);
    toolbar->addSeparator();
    toolbar->addAction(importMediaAction);
    toolbar->addSeparator();

    auto *projectName = new QLabel("  Untitled Project  ");
    projectName->setStyleSheet("color: #aeb7c6; font-weight: 600;");
    toolbar->addWidget(projectName);
}

void MainWindow::createCentralWorkspace()
{
    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(10, 9, 10, 8);
    root->setSpacing(8);

    auto *workspaceBar = new QHBoxLayout();
    workspaceBar->setSpacing(5);
    const QStringList modes = {"EDIT", "ANIMATION", "IMAGE", "FFMPEG"};
    for (int i = 0; i < modes.size(); ++i) {
        auto *button = new QToolButton(central);
        button->setText(modes.at(i));
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setMinimumWidth(105);
        if (i == 0) button->setChecked(true);
        workspaceBar->addWidget(button);
        connect(button, &QToolButton::clicked, this, [this, mode = modes.at(i)] {
            statusBar()->showMessage(mode + " workspace selected — engine is under development.", 3500);
        });
    }
    workspaceBar->addStretch();
    auto *layoutLabel = new QLabel("WORKSPACE", central);
    layoutLabel->setObjectName("mutedLabel");
    workspaceBar->addWidget(layoutLabel);
    root->addLayout(workspaceBar);

    auto *previewPanel = new QFrame(central);
    previewPanel->setObjectName("panelSurface");
    auto *previewLayout = new QVBoxLayout(previewPanel);
    previewLayout->setContentsMargins(12, 10, 12, 12);
    previewLayout->setSpacing(9);

    auto *previewHeader = new QHBoxLayout();
    previewHeader->addWidget(makeSectionLabel("VIEWER", previewPanel));
    previewHeader->addStretch();
    auto *fitButton = new QPushButton("Fit", previewPanel);
    auto *zoomButton = new QPushButton("100%", previewPanel);
    fitButton->setMaximumWidth(65);
    zoomButton->setMaximumWidth(75);
    previewHeader->addWidget(fitButton);
    previewHeader->addWidget(zoomButton);
    previewLayout->addLayout(previewHeader);

    m_previewStack = new QStackedWidget(previewPanel);
    m_previewLabel = new QLabel(
        "CREATORFORGE\n\nPreview will appear here\nAdd a video clip to begin", m_previewStack);
    m_previewLabel->setObjectName("previewCanvas");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumSize(420, 250);

    m_videoWidget = new QVideoWidget(m_previewStack);
    m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    m_previewStack->addWidget(m_previewLabel);
    m_previewStack->addWidget(m_videoWidget);
    m_previewStack->setCurrentWidget(m_previewLabel);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    previewLayout->addWidget(m_previewStack, 1);

    m_positionSlider = new QSlider(Qt::Horizontal, previewPanel);
    m_positionSlider->setRange(0, 0);
    m_positionSlider->setEnabled(false);
    previewLayout->addWidget(m_positionSlider);

    auto *transport = new QHBoxLayout();
    transport->addStretch();
    auto *previousButton = new QPushButton("|◀", previewPanel);
    auto *playButton = new QPushButton("▶", previewPanel);
    auto *nextButton = new QPushButton("▶|", previewPanel);
    previousButton->setFixedWidth(42);
    playButton->setFixedWidth(48);
    nextButton->setFixedWidth(42);
    transport->addWidget(previousButton);
    transport->addWidget(playButton);
    transport->addWidget(nextButton);
    m_videoAudioButton = new QPushButton("Video Audio: ON", previewPanel);
    m_videoAudioButton->setCheckable(true);
    m_videoAudioButton->setChecked(false);
    m_videoAudioButton->setToolTip("Mute or unmute audio embedded in the video");
    transport->addWidget(m_videoAudioButton);
    transport->addStretch();
    m_timecodeLabel = new QLabel("00:00:00.000", previewPanel);
    m_timecodeLabel->setStyleSheet("color: #aeb7c6; font-family: monospace; padding-right: 8px;");
    transport->addWidget(m_timecodeLabel);
    previewLayout->addLayout(transport);

    connect(m_videoAudioButton, &QPushButton::toggled, this,
            [this](bool muted) { setVideoAudioMuted(muted); });

    connect(playButton, &QPushButton::clicked, this, [this] {
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState)
            m_mediaPlayer->pause();
        else if (!m_mediaPlayer->source().isEmpty())
            m_mediaPlayer->play();
        else
            statusBar()->showMessage("Add a video clip first.", 3000);
    });
    connect(previousButton, &QPushButton::clicked, this, [this] {
        m_mediaPlayer->setPosition(0);
    });
    connect(nextButton, &QPushButton::clicked, this, [this] {
        m_mediaPlayer->setPosition(m_mediaPlayer->duration());
    });
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this,
            [playButton](QMediaPlayer::PlaybackState state) {
        playButton->setText(state == QMediaPlayer::PlayingState ? "Ⅱ" : "▶");
    });
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        m_positionSlider->setRange(0, static_cast<int>(qMin<qint64>(duration, 2147483647)));
        m_positionSlider->setEnabled(duration > 0);
    });
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        if (!m_isSeeking)
            m_positionSlider->setValue(static_cast<int>(position));
        const QString time = formatTimecode(position);
        m_timecodeLabel->setText(time);
        if (m_timelineTimeLabel)
            m_timelineTimeLabel->setText(time);
        if (m_timelineWidget)
            m_timelineWidget->setPlayheadSeconds(position / 1000.0);
    });
    connect(m_positionSlider, &QSlider::sliderPressed, this, [this] {
        m_isSeeking = true;
    });
    connect(m_positionSlider, &QSlider::sliderMoved, this, [this](int position) {
        m_mediaPlayer->setPosition(position);
    });
    connect(m_positionSlider, &QSlider::sliderReleased, this, [this] {
        m_mediaPlayer->setPosition(m_positionSlider->value());
        m_isSeeking = false;
    });
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &errorText) {
        m_previewStack->setCurrentWidget(m_previewLabel);
        m_previewLabel->setText("Unable to play this media.\n\n" + errorText);
        statusBar()->showMessage("Media playback error: " + errorText, 8000);
    });

    root->addWidget(previewPanel, 1);
    setCentralWidget(central);
}

void MainWindow::createMediaBrowser()
{
    auto *dock = new QDockWidget("MEDIA POOL", this);
    dock->setObjectName("MediaBrowserDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setMinimumWidth(210);

    auto *container = new QWidget(dock);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(9, 10, 9, 9);
    layout->setSpacing(8);

    auto *top = new QHBoxLayout();
    top->addWidget(makeSectionLabel("PROJECT MEDIA", container));
    top->addStretch();
    auto *addButton = new QPushButton("+", container);
    addButton->setFixedSize(30, 28);
    top->addWidget(addButton);
    layout->addLayout(top);

    m_mediaList = new QListWidget(container);
    m_mediaList->setAlternatingRowColors(true);
    m_mediaList->setSpacing(2);
    layout->addWidget(m_mediaList, 1);

    connect(m_mediaList, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem *item) {
        const QString path = item->data(Qt::UserRole).toString();
        const QString suffix = QFileInfo(path).suffix().toLower();
        const QStringList videoTypes = {"mp4", "mkv", "mov", "webm", "avi", "m4v"};
        if (!path.isEmpty() && videoTypes.contains(suffix))
            setPreviewMedia(path);
    });

    auto *hint = new QLabel("Media imported here is currently session-only.", container);
    hint->setObjectName("mutedLabel");
    hint->setWordWrap(true);
    layout->addWidget(hint);

    connect(addButton, &QPushButton::clicked, this, [this] {
        const QStringList paths = QFileDialog::getOpenFileNames(this, "Import Media");
        for (const QString &path : paths) {
            m_mediaList->addItem(QFileInfo(path).fileName());
        }
    });

    dock->setWidget(container);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::createPropertiesPanel()
{
    auto *dock = new QDockWidget("INSPECTOR", this);
    dock->setObjectName("PropertiesDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setMinimumWidth(230);

    auto *container = new QWidget(dock);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(9, 10, 9, 9);
    layout->setSpacing(9);

    layout->addWidget(makeSectionLabel("SELECTED OBJECT", container));
    auto *selected = new QLabel("Nothing selected", container);
    selected->setObjectName("mutedLabel");
    layout->addWidget(selected);

    m_propertiesTree = new QTreeWidget(container);
    m_propertiesTree->setColumnCount(2);
    m_propertiesTree->setHeaderLabels({"PROPERTY", "VALUE"});
    m_propertiesTree->setRootIsDecorated(false);
    m_propertiesTree->setAlternatingRowColors(true);

    auto *transform = new QTreeWidgetItem(m_propertiesTree, {"Transform", ""});
    transform->setExpanded(true);
    new QTreeWidgetItem(transform, {"Position X", "0"});
    new QTreeWidgetItem(transform, {"Position Y", "0"});
    new QTreeWidgetItem(transform, {"Scale X", "100%"});
    new QTreeWidgetItem(transform, {"Scale Y", "100%"});
    new QTreeWidgetItem(transform, {"Rotation", "0°"});

    auto *compositing = new QTreeWidgetItem(m_propertiesTree, {"Compositing", ""});
    compositing->setExpanded(true);
    new QTreeWidgetItem(compositing, {"Opacity", "100%"});
    new QTreeWidgetItem(compositing, {"Blend Mode", "Normal"});

    layout->addWidget(m_propertiesTree, 1);
    auto *note = new QLabel("Properties become editable when an engine is connected.", container);
    note->setObjectName("mutedLabel");
    note->setWordWrap(true);
    layout->addWidget(note);

    dock->setWidget(container);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::createTimelinePanel()
{
    auto *dock = new QDockWidget("TIMELINE", this);
    dock->setObjectName("TimelineDock");
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    dock->setMinimumHeight(225);

    auto *container = new QWidget(dock);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(10, 8, 10, 9);
    layout->setSpacing(7);

    auto *toolbar = new QHBoxLayout();
    toolbar->addWidget(makeSectionLabel("SEQUENCE 01", container));
    toolbar->addSpacing(12);

    m_timelineTimeLabel = new QLabel("00:00:00.000", container);
    m_timelineTimeLabel->setStyleSheet("font-family: monospace; color: #c4ccd8;");
    toolbar->addWidget(m_timelineTimeLabel);
    toolbar->addStretch();

    auto *addVideoButton = new QPushButton("+ Video", container);
    auto *addSubtitleButton = new QPushButton("+ Subtitle", container);
    auto *addAudioButton = new QPushButton("+ Audio", container);
    auto *addSfxButton = new QPushButton("+ SFX", container);
    auto *addVideoTrackButton = new QPushButton("+ V Track", container);
    auto *addSubtitleTrackButton = new QPushButton("+ T Track", container);
    auto *addAudioTrackButton = new QPushButton("+ A Track", container);
    auto *addEffectsTrackButton = new QPushButton("+ SFX Track", container);
    auto *snapButton = new QPushButton("Snap: ON", container);
    snapButton->setCheckable(true);
    snapButton->setChecked(true);
    toolbar->addWidget(addVideoButton);
    toolbar->addWidget(addSubtitleButton);
    toolbar->addWidget(addAudioButton);
    toolbar->addWidget(addSfxButton);
    toolbar->addWidget(addVideoTrackButton);
    toolbar->addWidget(addSubtitleTrackButton);
    toolbar->addWidget(addAudioTrackButton);
    toolbar->addWidget(addEffectsTrackButton);
    toolbar->addWidget(snapButton);
    layout->addLayout(toolbar);

    m_timelineWidget = new TimelineWidget(container);
    m_timelineWidget->setSeekCallback([this](double seconds) {
        m_mediaPlayer->setPosition(static_cast<qint64>(seconds * 1000.0));
    });

    auto *scrollArea = new QScrollArea(container);
    scrollArea->setWidget(m_timelineWidget);
    scrollArea->setWidgetResizable(false);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { background: #171b22; border: 1px solid #303744; border-radius: 4px; }");
    layout->addWidget(scrollArea, 1);

    connect(addVideoButton, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, "Add Video Clip", {},
            "Video Files (*.mp4 *.mkv *.mov *.webm *.avi *.m4v);;All Files (*)");
        if (path.isEmpty())
            return;

        double mediaDuration = probeMediaDuration(path);
        bool accepted = false;
        if (mediaDuration <= 0.0) {
            mediaDuration = QInputDialog::getDouble(
                this, "Video Duration",
                "Could not read duration with ffprobe. Enter duration (seconds):",
                10.0, 0.25, 86400.0, 2, &accepted);
            if (!accepted)
                return;
        }

        const double start = QInputDialog::getDouble(
            this, "Video Start", "Timeline start time (seconds):",
            0.0, 0.0, 86400.0, 2, &accepted);
        if (!accepted)
            return;

        const double duration = QInputDialog::getDouble(
            this, "Video Duration", "Clip duration (seconds):",
            mediaDuration, 0.25, mediaDuration, 2, &accepted);
        if (!accepted)
            return;

        m_timelineWidget->addClip(m_timelineWidget->lastTrackOfType(TimelineWidget::VideoTrack),
            QFileInfo(path).fileName(), start, duration, QColor("#3975b9"),
            path, mediaDuration);
        setPreviewMedia(path);
        statusBar()->showMessage("Video clip added to V1 and loaded in Viewer.", 3000);
    });

    connect(addSubtitleButton, &QPushButton::clicked, this, [this] {
        bool accepted = false;
        const QString text = QInputDialog::getText(
            this, "Add Subtitle", "Subtitle text:", QLineEdit::Normal, {}, &accepted);
        if (!accepted || text.trimmed().isEmpty())
            return;

        const double start = QInputDialog::getDouble(
            this, "Subtitle Start", "Start time (seconds):", 0.0, 0.0, 86400.0, 2, &accepted);
        if (!accepted)
            return;

        const double duration = QInputDialog::getDouble(
            this, "Subtitle Duration", "Duration (seconds):", 3.0, 0.25, 86400.0, 2, &accepted);
        if (!accepted)
            return;

        m_timelineWidget->addClip(m_timelineWidget->lastTrackOfType(TimelineWidget::SubtitleTrack),
                                  text, start, duration, QColor("#7654a8"));
        statusBar()->showMessage("Subtitle clip added to T1.", 3000);
    });

    connect(addAudioButton, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, "Add Audio Clip", {},
            "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a *.aac);;All Files (*)");
        if (path.isEmpty())
            return;

        const double mediaDuration = probeMediaDuration(path);
        bool accepted = false;
        const double start = QInputDialog::getDouble(
            this, "Audio Start", "Timeline start time (seconds):",
            0.0, 0.0, 86400.0, 2, &accepted);
        if (!accepted)
            return;

        const double defaultDuration = mediaDuration > 0.0 ? mediaDuration : 10.0;
        const double maximumDuration = mediaDuration > 0.0 ? mediaDuration : 86400.0;
        const double duration = QInputDialog::getDouble(
            this, "Audio Duration",
            mediaDuration > 0.0 ? "Clip duration (seconds):"
                               : "ffprobe unavailable. Enter clip duration (seconds):",
            defaultDuration, 0.25, maximumDuration, 2, &accepted);
        if (!accepted)
            return;

        m_timelineWidget->addClip(m_timelineWidget->lastTrackOfType(TimelineWidget::AudioTrack),
            QFileInfo(path).fileName(), start, duration, QColor("#287c68"),
            path, mediaDuration);
        statusBar()->showMessage("Audio clip added to A1.", 3000);
    });

    connect(addVideoTrackButton, &QPushButton::clicked, this, [this] {
        const int index = m_timelineWidget->addTrack(
            QString("V%1  Video").arg(m_timelineWidget->lastTrackOfType(TimelineWidget::VideoTrack) + 2),
            QColor("#3975b9"));
        statusBar()->showMessage(QString("Video track %1 added.").arg(index + 1), 2500);
    });
    connect(addSubtitleTrackButton, &QPushButton::clicked, this, [this] {
        const int index = m_timelineWidget->addTrack(
            QString("T%1  Subtitle").arg(m_timelineWidget->lastTrackOfType(TimelineWidget::SubtitleTrack) + 2), QColor("#7654a8"));
        statusBar()->showMessage(QString("Subtitle track %1 added.").arg(index + 1), 2500);
    });
    connect(addAudioTrackButton, &QPushButton::clicked, this, [this] {
        const int index = m_timelineWidget->addTrack(
            QString("A%1  Audio").arg(m_timelineWidget->lastTrackOfType(TimelineWidget::AudioTrack) + 2), QColor("#287c68"));
        statusBar()->showMessage(QString("Audio track %1 added.").arg(index + 1), 2500);
    });
    connect(addEffectsTrackButton, &QPushButton::clicked, this, [this] {
        const int index = m_timelineWidget->addTrack(
            QString("SFX%1  Effects").arg(m_timelineWidget->lastTrackOfType(TimelineWidget::EffectsTrack) + 1), QColor("#a47736"));
        statusBar()->showMessage(QString("Effects track %1 added.").arg(index + 1), 2500);
    });

    connect(addSfxButton, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, "Add Sound Effect", {},
            "Sound Files (*.wav *.mp3 *.flac *.ogg *.m4a *.aac);;All Files (*)");
        if (path.isEmpty())
            return;

        const double mediaDuration = probeMediaDuration(path);
        bool accepted = false;
        const double start = QInputDialog::getDouble(
            this, "Sound Effect Start", "Timeline start time (seconds):",
            0.0, 0.0, 86400.0, 2, &accepted);
        if (!accepted)
            return;

        const double defaultDuration = mediaDuration > 0.0 ? mediaDuration : 3.0;
        const double maximumDuration = mediaDuration > 0.0 ? mediaDuration : 86400.0;
        const double duration = QInputDialog::getDouble(
            this, "Sound Effect Duration", "Clip duration (seconds):",
            defaultDuration, 0.25, maximumDuration, 2, &accepted);
        if (!accepted)
            return;

        m_timelineWidget->addClip(
            m_timelineWidget->lastTrackOfType(TimelineWidget::EffectsTrack),
            QFileInfo(path).fileName(), start, duration, QColor("#a47736"),
            path, mediaDuration);
        statusBar()->showMessage("Sound effect clip added to the SFX track.", 3000);
    });

    connect(snapButton, &QPushButton::toggled, this, [this, snapButton](bool enabled) {
        snapButton->setText(enabled ? "Snap: ON" : "Snap: OFF");
        m_timelineWidget->setSnapEnabled(enabled);
    });

    dock->setWidget(container);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
}


void MainWindow::setVideoAudioMuted(bool muted)
{
    m_audioOutput->setMuted(muted);
    if (m_videoAudioButton)
        m_videoAudioButton->setText(muted ? "Video Audio: OFF" : "Video Audio: ON");
}

void MainWindow::setPreviewMedia(const QString &path)
{
    if (path.isEmpty())
        return;

    m_mediaPlayer->stop();
    m_mediaPlayer->setSource(QUrl::fromLocalFile(path));
    m_positionSlider->setValue(0);
    m_positionSlider->setRange(0, 0);
    m_previewStack->setCurrentWidget(m_videoWidget);
    m_timecodeLabel->setText("00:00:00.000");
    m_timelineTimeLabel->setText("00:00:00.000");
    statusBar()->showMessage("Loaded in Viewer: " + QFileInfo(path).fileName(), 4000);
}

} // namespace CreatorForge
