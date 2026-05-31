#include <QApplication>
#include "core/MainWindow.h"
#include "core/Constants.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用信息（用于QSettings等）
    app.setApplicationName(App::APP_NAME);
    app.setApplicationVersion(App::APP_VERSION);
    app.setOrganizationName(App::APP_ORG);

    // 创建并显示主窗口
    MainWindow window;
    window.show();

    return app.exec();
}
