/**
 * @file StartupOptions.h
 * @brief 应用启动参数解析结果，供 main.cpp 在窗口创建后执行轻量入口路由。
 */

#ifndef STARTUP_OPTIONS_H
#define STARTUP_OPTIONS_H

#include <QString>
#include <QStringList>

/**
 * @brief 解析 EmbedDebug 启动参数中的工作台直达请求。
 *
 * 仅保留应用协调层需要的稳定 panel id，不持有窗口或面板指针。
 * 当前支持 `--panel <id>`、`--station serial` 以及 Serial Station profile 启动档案参数。
 */
class StartupOptions {
public:
    StartupOptions() = default; ///< 构造空启动参数

    /**
     * @brief 从 QApplication 参数构建启动选项。
     * @param arguments QCoreApplication::arguments() 返回的完整参数列表
     * @return 解析后的启动选项；无法识别时返回空 panel id
     */
    static StartupOptions fromArguments(const QStringList& arguments);

    /**
     * @brief 获取启动后希望打开的稳定面板ID。
     * @return 面板ID；为空表示沿用会话恢复或默认入口
     */
    QString panelId() const;

    /**
     * @brief 获取启动时需要加载的 Serial Station 配置档案路径。
     * @return 档案路径；为空表示不加载启动档案
     */
    QString profileFilePath() const;

    /**
     * @brief 是否请求加载 Serial Station 最近一次成功使用的配置档案。
     * @return true 表示未提供显式档案路径时应尝试加载上次档案
     */
    bool loadLastProfile() const;

private:
    StartupOptions(const QString& panelId,
                   const QString& profileFilePath,
                   bool loadLastProfile); ///< 写入已标准化启动选项
    QString m_panelId; ///< 启动后需要打开的稳定面板ID
    QString m_profileFilePath; ///< 启动后需要加载的 Serial Station 档案路径
    bool m_loadLastProfile = false; ///< 是否加载最近一次成功使用的 Serial Station 档案
};

#endif // STARTUP_OPTIONS_H
