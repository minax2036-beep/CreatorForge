#pragma once

#include <QMainWindow>

class QLabel;
class QListWidget;
class QTreeWidget;
class QWidget;

namespace CreatorForge {

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

    QListWidget *m_mediaList = nullptr;
    QLabel *m_previewLabel = nullptr;
    QTreeWidget *m_propertiesTree = nullptr;
};

} // namespace CreatorForge
