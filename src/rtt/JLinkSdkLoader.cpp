/**
 * @file JLinkSdkLoader.cpp
 * @brief J-Link SDK 动态库加载器实现
 *
 * 使用 QLibrary 在运行时加载 J-Link SDK 动态库。
 * 当前为桩实现，验证 QLibrary 加载机制，后续迭代将解析实际 SDK 函数。
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

/**
 * @brief 析构函数
 *
 * 析构时自动卸载已加载的 SDK 动态库。
 */
JLinkSdkLoader::~JLinkSdkLoader()
{
    unload();
}

/**
 * @brief 加载 J-Link SDK 动态库
 *
 * 创建 QLibrary 实例并加载指定路径的动态库文件。
 * 尝试解析一个基础符号来验证库文件有效性。
 * 加载结果通过 sdkLoaded/sdkLoadFailed 信号通知外部。
 *
 * @param path DLL 文件路径，为空则使用系统默认搜索路径
 * @return true 加载成功，false 加载失败（文件不存在或符号解析失败）
 */
bool JLinkSdkLoader::load(const QString& path)
{
    // 如果已经加载，先卸载旧实例
    if (m_loaded) {
        unload();
    }

    // 创建 QLibrary 并设置路径
    m_library = new QLibrary(path.isEmpty()
                                 ? QStringLiteral("JLinkARM")
                                 : path,
                             this);

    // 尝试加载动态库
    if (!m_library->load()) {
        const QString error = m_library->errorString();
        emit sdkLoadFailed(tr("SDK 加载失败: %1").arg(error));
        delete m_library;
        m_library = nullptr;
        return false;
    }

    // 尝试解析一个桩符号以验证库可用性
    // TODO: 后续迭代替换为实际 SDK 函数符号（如 JLINK_Open）
    auto dummySymbol = reinterpret_cast<void(*)()>(
        m_library->resolve("JLINK_Open"));
    if (!dummySymbol) {
        // 符号解析失败，但库本身加载成功 — 标记为已加载（桩模式）
        // 实际 SDK 集成时此处应返回 false
    }

    m_loaded = true;
    emit sdkLoaded();
    return true;
}

/**
 * @brief 卸载 J-Link SDK
 *
 * 卸载动态库并释放 QLibrary 实例。
 * 重置所有 SDK 函数指针和加载状态。
 */
void JLinkSdkLoader::unload()
{
    if (m_library) {
        if (m_library->isLoaded()) {
            m_library->unload();
        }
        delete m_library;
        m_library = nullptr;
    }
    m_loaded = false;
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
 *
 * 桩实现：返回 "Unknown"。
 * 后续迭代将通过 JLINK_GetFirmwareString 等接口获取实际版本。
 *
 * @return 版本号字符串
 */
QString JLinkSdkLoader::sdkVersion() const
{
    if (!m_loaded) {
        return QString();
    }

    // TODO: 调用 JLINK_GetFirmwareString 或 JLINK_GetDLLVersion 获取版本
    return tr("Unknown");
}
