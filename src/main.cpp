#include <QApplication>
#include <QTranslator>
#include <QFileInfo>
#include <QDir>
#include "core/MainWindow.h"
#include "core/Constants.h"
#include "utils/SettingsManager.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用信息（用于QSettings等）
    app.setApplicationName(App::APP_NAME);
    app.setApplicationVersion(App::APP_VERSION);
    app.setOrganizationName(App::APP_ORG);

    // 加载翻译文件
    QTranslator translator;
    QString langCode = SettingsManager::instance().loadLanguage();
    if (langCode != Language::CHINESE) {
        QString qmFile = QStringLiteral("embeddebug_%1.qm").arg(langCode);
        // 搜索路径: 应用所在目录/translations/
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList searchPaths = {
            appDir + QStringLiteral("/translations/") + qmFile,
            appDir + QStringLiteral("/../resources/translations/") + qmFile,
        };
        for (const QString& path : searchPaths) {
            if (QFileInfo::exists(path) && translator.load(path)) {
                app.installTranslator(&translator);
                break;
            }
        }
    }

    // 创建并显示主窗口
    MainWindow window;
    window.show();

    return app.exec();
}
