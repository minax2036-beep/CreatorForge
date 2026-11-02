#include "MainWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace CreatorForge {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("CreatorForge 0.1.0");
    resize(1440, 900);
    setMinimumSize(960, 640);

    createActions();
    createCentralWorkspace();
    createMediaBrowser();
    createPropertiesPanel();
    createTimelinePanel();

    statusBar()->showMessage("Ready — CreatorForge foundation");
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
            "Project creation will be implemented in the project manager milestone.");
    });

    connect(openProjectAction, &QAction::triggered, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, "Open CreatorForge Project", {},
            "CreatorForge Project (*.cforge);;All Files (*)");
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
            statusBar()->showMessage(
                QString("Added %1 item(s) to the temporary media list").arg(paths.size()), 5000);
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
            "<h2>CreatorForge 0.1.0</h2>"
            "<p>A unified creative suite for video editing, animation, "
            "image compositing and media processing.</p>"
            "<p>Early development foundation.</p>");
    });

    auto *toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->addAction(newProjectAction);
    toolbar->addAction(openProjectAction);
    toolbar->addAction(importMediaAction);
}

void MainWindow::createCentralWorkspace()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    m_workspaceTabs = new QTabWidget(central);
    m_workspaceTabs->setDocumentMode(true);

    const QStringList workspaces = {
        "Video", "Animation", "Image", "FFmpeg"
    };

    for (const QString &name : workspaces) {
        auto *page = new QWidget(m_workspaceTabs);
        auto *pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(0, 0, 0, 0);

        auto *description = new QLabel(
            QString("%1 workspace\n\nThe %1 engine is planned for a future milestone.")
                .arg(name), page);
        description->setAlignment(Qt::AlignCenter);
        description->setObjectName("workspacePlaceholder");
        pageLayout->addWidget(description);

        m_workspaceTabs->addTab(page, name);
    }

    layout->addWidget(m_workspaceTabs, 1);
    setCentralWidget(central);
}

void MainWindow::createMediaBrowser()
{
    auto *dock = new QDockWidget("Media Browser", this);
    dock->setObjectName("MediaBrowserDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *container = new QWidget(dock);
    auto *layout = new QVBoxLayout(container);

    auto *hint = new QLabel("Imported media for this session");
    hint->setWordWrap(true);

    m_mediaList = new QListWidget(container);
    m_mediaList->setAlternatingRowColors(true);

    auto *importButton = new QPushButton("Import Media...");
    connect(importButton, &QPushButton::clicked, this, [this] {
        const QStringList paths = QFileDialog::getOpenFileNames(this, "Import Media");
        for (const QString &path : paths) {
            m_mediaList->addItem(QFileInfo(path).fileName());
        }
    });

    layout->addWidget(hint);
    layout->addWidget(m_mediaList, 1);
    layout->addWidget(importButton);

    dock->setWidget(container);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::createPropertiesPanel()
{
    auto *dock = new QDockWidget("Properties", this);
    dock->setObjectName("PropertiesDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_propertiesTree = new QTreeWidget(dock);
    m_propertiesTree->setColumnCount(2);
    m_propertiesTree->setHeaderLabels({"Property", "Value"});
    m_propertiesTree->setRootIsDecorated(false);

    auto *general = new QTreeWidgetItem(m_propertiesTree, {"General", ""});
    general->setExpanded(true);
    new QTreeWidgetItem(general, {"Selection", "Nothing selected"});
    new QTreeWidgetItem(general, {"Position", "—"});
    new QTreeWidgetItem(general, {"Scale", "—"});
    new QTreeWidgetItem(general, {"Opacity", "—"});

    dock->setWidget(m_propertiesTree);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::createTimelinePanel()
{
    auto *dock = new QDockWidget("Timeline", this);
    dock->setObjectName("TimelineDock");
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

    m_timelinePanel = new QWidget(dock);
    auto *layout = new QVBoxLayout(m_timelinePanel);

    auto *toolbar = new QHBoxLayout();
    toolbar->addWidget(new QLabel("00:00:00:00"));
    toolbar->addStretch();
    toolbar->addWidget(new QLabel("Timeline foundation — editing is not available yet"));

    layout->addLayout(toolbar);

    for (const QString &trackName : {"Video 1", "Video 2", "Audio 1"}) {
        auto *row = new QHBoxLayout();
        auto *name = new QLabel(trackName);
        name->setMinimumWidth(90);

        auto *track = new QLabel(" ");
        track->setMinimumHeight(28);
        track->setStyleSheet(
            "QLabel { background: #30394b; border: 1px solid #46536b; border-radius: 3px; }");

        row->addWidget(name);
        row->addWidget(track, 1);
        layout->addLayout(row);
    }

    dock->setWidget(m_timelinePanel);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    dock->setMinimumHeight(150);
}

} // namespace CreatorForge
