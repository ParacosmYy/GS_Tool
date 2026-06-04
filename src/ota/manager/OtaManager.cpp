/**
 * @file OtaManager.cpp
 * @brief OTA升级管理器实现 - 构造析构、信号连接、连接管理、传输控制
 *
 * 核心流程:
 *   startTransfer()
 *     1. 设置状态为 Selecting
 *     2. 验证文件路径(存在/可读/大小)
 *     3. 检测文件类型(BIN/HEX)
 *     4. HEX文件自动转BIN(IntelHexParser)
 *     5. 设置状态为 Transferring
 *     6. 启动传输计时器
 *     7. 调用对应协议的start()
 *
 *   传输完成时记录速率到历史记录，用于计算平均速率
 */

#include "ota/manager/OtaManager.h"
#include "protocol/hex/IntelHexParser.h"

#include <QFileInfo>
#include <QFile>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造OTA管理器，创建三个传输协议实例并连接信号 */
OtaManager::OtaManager(QObject* parent)
    : QObject(parent)
    , m_xmodem(new XModemTransfer(this))
    , m_ymodem(new YModemTransfer(this))
    , m_zmodem(new ZModemTransfer(this))
{
    // 连接三个协议的基础信号(progress/complete/error)
    connectTransferSignals(m_xmodem);
    connectTransferSignals(m_ymodem);
    connectTransferSignals(m_zmodem);

    // 连接各协议的传输速率信号
    connectXModemStats();
    connectYModemStats();

    // 连接XModem模式降级信号
    connect(m_xmodem, &XModemTransfer::modeDegraded,
            this, [this](const QString& from, const QString& to) {
                QString msg = tr("接收方仅支持Checksum模式，已自动从 %1 降级为 %2")
                                  .arg(from, to);
                qWarning() << "OtaManager:" << msg;
                emit modeDegraded(msg);
            });
}

/** @brief 析构函数 — 清理HEX转换产生的临时BIN文件 */
OtaManager::~OtaManager()
{
    delete m_tempBinFile;
    m_tempBinFile = nullptr;
    if (!m_tempBinPath.isEmpty()) {
        QFile::remove(m_tempBinPath);
    }
}

// 信号连接/传输控制 → 已拆分至 OtaManagerSignals.cpp

// ---- 状态管理/状态查询/校验和验证 → 已拆分至 OtaManagerVerify.cpp ----
// ---- 文件验证/类型检测/HEX转换 → 已拆分至 OtaManagerFileOps.cpp ----
// ---- 协议显示名称 / 传输统计 / 平均速率 → 已拆分至 OtaManagerProgress.cpp ----
