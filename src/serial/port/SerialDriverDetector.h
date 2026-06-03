/**
 * @file SerialDriverDetector.h
 * @brief 串口驱动检测器 - 检测系统中已安装的USB转串口驱动芯片
 *
 * 职责:
 *   1. 检测常见USB转串口芯片的驱动是否已安装(CH340/CP2102/FT232/PL2303)
 *   2. 提供驱动状态摘要，用于启动时的用户提示
 *   3. 纯静态方法+静态统计计数器，可随时调用
 *
 * 检测方式: 遍历QSerialPortInfo::availablePorts()，匹配设备描述符中的芯片关键词
 *
 * 协作关系:
 *   - SerialConfigPanel: 启动时调用检测，无驱动时显示安装提示
 *   - MainWindow: 初始化时检查驱动状态
 */

#ifndef SERIALDRIVERDETECTOR_H
#define SERIALDRIVERDETECTOR_H

#include <QStringList>
#include <QVector>
#include <QtGlobal>

/**
 * @brief 单个驱动的检测结果
 */
struct DriverInfo {
    QString driverName;    ///< 驱动芯片名称，如"CH340"、"CP2102"
    QString description;   ///< 系统中的设备描述，如"WCH CH340 Serial Adapter"
    bool installed = false; ///< 是否检测到该驱动已安装
};

/**
 * @brief 串口驱动检测器 - 纯静态工具类(含扫描统计)
 *
 * 使用示例:
 * @code
 *   if (!SerialDriverDetector::hasAnyDriverInstalled()) {
 *       qWarning() << SerialDriverDetector::driverStatusSummary();
 *   }
 *   // 查看扫描统计
 *   qDebug() << "scans:" << SerialDriverDetector::totalScans()
 *            << "drivers found:" << SerialDriverDetector::totalDriversFound();
 * @endcode
 */
class SerialDriverDetector {
public:
    /**
     * @brief 检测所有已知USB转串口驱动的安装状态
     * @return DriverInfo向量，每个元素对应一种已知芯片的检测结果
     *
     * 检测的芯片: CH340/CH910/CP210/FTDI/FT232/PL2303/Silicon Labs/Prolific/WCH
     */
    static QVector<DriverInfo> detectDrivers();

    /**
     * @brief 快速检查是否有任何已知驱动已安装
     * @return true=至少检测到一种已知驱动，false=未检测到
     */
    static bool hasAnyDriverInstalled();

    /**
     * @brief 获取人类可读的驱动状态摘要（中文）
     * @return 多行文本，包含检测到的驱动列表和可用端口数
     *
     * 三种情况:
     *   1. 无串口设备 → 提示检查USB连接和驱动安装
     *   2. 有已知驱动 → 列出已检测到的驱动和端口数
     *   3. 有端口但无匹配驱动 → 提示可能是原生COM口或未识别的适配器
     */
    static QString driverStatusSummary();

    // ---- 扫描统计 ----

    /** @brief 获取累计扫描次数 @return 自上次重置以来的驱动扫描总次数 */
    static quint64 totalScans() { return m_totalScans; }

    /** @brief 获取累计检测到的驱动数(所有扫描中发现的已安装驱动总数) @return 累计已安装驱动计数 */
    static quint64 totalDriversFound() { return m_totalDriversFound; }

    /** @brief 重置扫描统计计数器(totalScans/totalDriversFound归零) */
    static void resetStats();

private:
    /** @brief 已知USB转串口驱动芯片的关键词列表，用于匹配设备描述符 */
    static const QStringList kKnownDrivers;

    // ---- 扫描统计计数器 ----
    static quint64 m_totalScans;       ///< 累计驱动扫描次数(每次调用detectDrivers递增)
    static quint64 m_totalDriversFound;///< 累计检测到的已安装驱动总数
};

#endif // SERIALDRIVERDETECTOR_H
