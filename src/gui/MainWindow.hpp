#pragma once

#include <QMainWindow>

class QComboBox;
class QLabel;
class QListWidget;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace CreatorForge {

class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createActions();
    void createCentralWorkspace();
    void createMediaBrowser();
    void createPreviewPanel();
    void createPropertiesPanel();
    void createTimelinePanel();
    void setActiveWorkspace(const QString &workspace);

    QTabWidget *m_workspaceTabs = nullptr;
    QListWidget *m_mediaList = nullptr;
    QLabel *m_previewLabel = nullptr;
    QTreeWidget *m_propertiesTree = nullptr;
    QWidget *m_timelinePanel = nullptr;
    QLabel *m_statusLabel = nullptr;
};

} // namespace CreatorForge
