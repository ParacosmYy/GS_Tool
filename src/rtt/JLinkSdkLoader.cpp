/**
 * @file JLinkSdkLoader.cpp
 * @brief J-Link SDK 动态库加载器实现 — 骨架文件
 */

#include "rtt/JLinkSdkLoader.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
JLinkSdkLoader::JLinkSdkLoader(QObject* parent)
    : QObject(parent)
    , m_library(nullptr)
    , m_loaded(false)
{
}

/** @brief 析构函数，自动卸载 SDK */
JLinkSdkLoader::~JLinkSdkLoader()
{
    unload();
}

/**
 * @brief 加载 J-Link SDK 动态库
 *
 * 使用 QLibrary 加载指定路径的 DLL 文件，
 * 并解析所需的 SDK 函数符号。
 *
 * @param path DLL 文件路径
 * @return true 加载成功，false 加载失败
 */
bool JLinkSdkLoader::load(const QString& path)
{
    Q_UNUSED(path)
    // TODO: 使用 QLibrary 加载 JLinkARM.dll
    // TODO: 解析 SDK 函数符号（JLINK_Open, JLINK_Connect 等）
    return false;
}

/**
 * @brief 卸载 J-Link SDK
 *
 * 释放 SDK 函数指针并卸载动态库。
 */
void JLinkSdkLoader::unload()
{
    // TODO: 释放资源，卸载动态库
}

/**
 * @brief 查询 SDK 是否已加载
 * @return true 已加载，false 未加载
 */
bool JLinkSdkLoader::isLoaded() const
{
    return m_loaded;
}

/**
 * @brief 获取 SDK 版本字符串
 * @return 版本号，未加载时返回空字符串
 */
QString JLinkSdkLoader::sdkVersion() const
{
    // TODO: 调用 JLINK_GetFirmwareString 或类似接口获取版本
    return QString();
}
