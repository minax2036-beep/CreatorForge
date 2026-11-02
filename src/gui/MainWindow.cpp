#include "MainWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

namespace CreatorForge {
namespace {

QLabel *makeSectionLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName("sectionLabel");
    return label;
}

QFrame *makeTrack(const QString &name, const QString &color, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName("timelineClip");
    frame->setMinimumHeight(28);
    frame->setStyleSheet(QString(
        "QFrame#timelineClip { background: %1; border: 1px solid %2; border-radius: 4px; }")
        .arg(color, color));
    return frame;
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

    createActions();
    createCentralWorkspace();
    createMediaBrowser();
    createPropertiesPanel();
    createTimelinePanel();

    statusBar()->showMessage("Ready  •  CreatorForge 0.1.0");
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
            m_mediaList->addItem(QFileInfo(path).fileName());
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

    m_previewLabel = new QLabel(
        "CREATORFORGE\n\nPreview will appear here\nImport media to begin", previewPanel);
    m_previewLabel->setObjectName("previewCanvas");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumSize(420, 250);
    previewLayout->addWidget(m_previewLabel, 1);

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
    transport->addStretch();
    auto *timecode = new QLabel("00:00:00:00", previewPanel);
    timecode->setStyleSheet("color: #aeb7c6; font-family: monospace; padding-right: 8px;");
    transport->addWidget(timecode);
    previewLayout->addLayout(transport);

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
    dock->setMinimumHeight(175);

    auto *container = new QWidget(dock);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(10, 8, 10, 9);
    layout->setSpacing(7);

    auto *toolbar = new QHBoxLayout();
    toolbar->addWidget(makeSectionLabel("SEQUENCE 01", container));
    toolbar->addSpacing(12);
    auto *time = new QLabel("00:00:00:00", container);
    time->setStyleSheet("font-family: monospace; color: #c4ccd8;");
    toolbar->addWidget(time);
    toolbar->addStretch();
    toolbar->addWidget(new QLabel("Snap", container));
    auto *snapButton = new QPushButton("ON", container);
    snapButton->setCheckable(true);
    snapButton->setChecked(true);
    snapButton->setFixedWidth(48);
    toolbar->addWidget(snapButton);
    layout->addLayout(toolbar);

    const QStringList tracks = {"V1  Video", "V2  Overlay", "A1  Audio"};
    const QStringList colors = {"#315d86", "#65518c", "#39715f"};
    for (int i = 0; i < tracks.size(); ++i) {
        auto *row = new QHBoxLayout();
        row->setSpacing(8);
        auto *name = new QLabel(tracks.at(i), container);
        name->setMinimumWidth(105);
        name->setStyleSheet("color: #aeb7c6;");
        row->addWidget(name);
        row->addWidget(makeTrack(tracks.at(i), colors.at(i), container), 1);
        layout->addLayout(row);
    }

    dock->setWidget(container);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
}

} // namespace CreatorForge
