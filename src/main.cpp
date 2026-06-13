/**
 * @file main.cpp
 * @brief 应用程序入口 — 初始化Qt应用、加载翻译、创建主窗口
 *
 * 启动流程:
 *   1. 创建QApplication实例
 *   2. 加载Qt翻译(对话框等标准组件中文化)
 *   3. 加载应用翻译(EmbedDebug自身的tr()翻译)
 *   4. 创建MainWindow并显示
 */
#include <QApplication>
#include <QTranslator>
#include <QFileInfo>
#include <QDir>
#include "core/mainwindow/MainWindow.h"
#include "core/mainwindow/StartupOptions.h"
#include "shared/AppConstants.h"
#include "utils/settings/SettingsManager.h"

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
    const StartupOptions startupOptions = StartupOptions::fromArguments(QCoreApplication::arguments());
    MainWindow window;
    window.show();
    window.applyStartupOptions(startupOptions);

    return app.exec();
}
