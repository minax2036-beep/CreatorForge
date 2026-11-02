#include "gui/MainWindow.hpp"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QApplication::setApplicationName("CreatorForge");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("CreatorForge");

    if (QStyle *style = QStyleFactory::create("Fusion")) {
        application.setStyle(style);
    }

    CreatorForge::MainWindow window;
    window.show();

    return application.exec();
}
