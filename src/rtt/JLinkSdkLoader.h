/**
 * @file JLinkSdkLoader.h
 * @brief J-Link SDK 动态库加载器 — 运行时加载 J-Link SDK DLL
 *
 * 使用 QLibrary 动态加载 J-Link SDK，避免编译期硬依赖。
 * 提供 SDK 函数指针的获取和生命周期管理。
 *
 * 协作关系:
 *   - JLinkRttConnection: 通过本加载器获取 SDK 函数
 *   - RttConfigPanel: 显示 SDK 加载状态
 */
#ifndef JLINKSDKLOADER_H
#define JLINKSDKLOADER_H

#include <QObject>
#include <QLibrary>

/**
 * @brief J-Link SDK 动态库加载器
 *
 * 负责在运行时加载 JLinkARM.dll（或对应平台库），
 * 解析 SDK 函数符号并缓存函数指针。
 */
class JLinkSdkLoader : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit JLinkSdkLoader(QObject* parent = nullptr);

    /** @brief 析构函数，自动卸载 SDK */
    ~JLinkSdkLoader() override;

    /**
     * @brief 加载 J-Link SDK 动态库
     *
     * 使用 QLibrary 加载指定路径的 DLL，并尝试解析一个基础符号
     * 以验证库的有效性。加载成功后发出 sdkLoaded 信号。
     *
     * @param path DLL 文件路径，为空则使用系统搜索路径
     * @return true 加载成功，false 加载失败
     */
    bool load(const QString& path = QString());

    /**
     * @brief 卸载 J-Link SDK
     *
     * 释放动态库句柄，重置加载状态。
     */
    void unload();

    /**
     * @brief 查询 SDK 是否已加载
     * @return true 已加载，false 未加载
     */
    bool isLoaded() const;

    /**
     * @brief 获取 SDK 版本字符串
     * @return 版本号，未加载时返回空字符串
     */
    QString sdkVersion() const;

signals:
    /** @brief SDK 加载成功信号 */
    void sdkLoaded();

    /**
     * @brief SDK 加载失败信号
     * @param error 错误描述
     */
    void sdkLoadFailed(const QString& error);

private:
    QLibrary* m_library = nullptr;  ///< 动态库句柄
    bool m_loaded = false;          ///< 加载状态标志
};

#endif // JLINKSDKLOADER_H
