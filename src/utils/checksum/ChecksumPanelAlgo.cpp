/**
 * @file ChecksumPanelAlgo.cpp
 * @brief 校验和面板 — 数据解析、算法查询、历史格式化与统计接口
 * @author Serial Tool Team
 * @date 2026-06-04
 *
 * 从 ChecksumPanel.cpp 拆分而来，包含：
 *   - 输入数据解析（十六进制 / ASCII / 文件）
 *   - 算法与结果查询
 *   - 历史记录格式化
 *   - 面板统计计数器接口
 *
 * @see ChecksumPanel.cpp — 构造函数、UI 槽函数、拖放事件
 */

#include "utils/checksum/ChecksumPanel.h"

#include <QFile>

// ---- 数据解析 ----

/**
 * @brief 根据输入模式解析输入数据为字节数组
 */
QByteArray ChecksumPanel::inputData() const
{
    int mode = m_inputModeCombo->currentData().toInt();

    switch (mode) {
    case 0: {
        // 十六进制模式
        QString text = m_inputEdit->toPlainText().simplified();
        text.remove(' ');
        return QByteArray::fromHex(text.toUtf8());
    }
    case 1: {
        // ASCII文本模式
        return m_inputEdit->toPlainText().toUtf8();
    }
    case 2: {
        // 二进制文件模式：输入框内容为文件路径
        QString path = m_inputEdit->toPlainText().trimmed();
        QFile file(path);
        // 限制文件大小为16MB，防止大文件导致OOM
        static const qint64 kMaxChecksumFileSize = 16 * 1024 * 1024;
        if (file.open(QIODevice::ReadOnly)) {
            if (file.size() > kMaxChecksumFileSize) {
                file.close();
                return QByteArray();
            }
            return file.readAll();
        }
        return QByteArray();
    }
    default:
        return QByteArray();
    }
}

// ---- 算法与结果查询 ----

/**
 * @brief 获取当前选中算法
 */
ChecksumCalculator::Algorithm ChecksumPanel::selectedAlgorithm() const
{
    return static_cast<ChecksumCalculator::Algorithm>(
        m_algoCombo->currentData().toInt());
}

/**
 * @brief 返回最近一次计算结果
 */
quint64 ChecksumPanel::result() const
{
    return m_result;
}

// ---- 历史记录格式化 ----

/**
 * @brief 添加计算结果到历史记录列表
 *
 * @param value 校验和值
 * @param algoName 算法名称
 * @param inputHex 输入数据十六进制（截断显示）
 */
void ChecksumPanel::addHistoryEntry(quint64 value, const QString& algoName,
                                    const QString& inputHex)
{
    /* 根据结果位宽选择格式 */
    QString hexResult;
    if (value <= 0xFF) {
        hexResult = QStringLiteral("%1").arg(value, 2, 16, QLatin1Char('0'));
    } else if (value <= 0xFFFF) {
        hexResult = QStringLiteral("%1").arg(value, 4, 16, QLatin1Char('0'));
    } else {
        hexResult = QStringLiteral("%1").arg(value, 8, 16, QLatin1Char('0'));
    }
    hexResult = hexResult.toUpper();

    QString displayText = tr("[%1] %2 ← 0x%3")
                              .arg(algoName)
                              .arg(hexResult)
                              .arg(inputHex);

    auto* historyItem = new QListWidgetItem(displayText, m_historyList);
    historyItem->setData(Qt::UserRole, value);

    /* 限制历史记录最多50条 */
    while (m_historyList->count() > 50) {
        delete m_historyList->takeItem(0);
    }
}

// ---- 统计计数接口 ----

/**
 * @brief 获取累计计算次数（面板层面）
 */
quint64 ChecksumPanel::totalCalculations() const
{
    return m_totalCalculations;
}

/**
 * @brief 获取累计复制到剪贴板次数
 */
quint64 ChecksumPanel::totalCopyActions() const
{
    return m_totalCopyActions;
}

/**
 * @brief 获取累计算法切换次数
 */
quint64 ChecksumPanel::totalAlgorithmChanges() const
{
    return m_totalAlgorithmChanges;
}

/**
 * @brief 获取累计复制结果到剪贴板操作次数
 */
quint64 ChecksumPanel::totalCopyToClipboard() const
{
    return m_totalCopyToClipboard;
}

/**
 * @brief 获取累计输入内容更新次数
 */
quint64 ChecksumPanel::totalInputUpdates() const
{
    return m_totalInputUpdates;
}

/**
 * @brief 获取累计输入模式切换次数(十六进制/ASCII/文件)
 */
quint64 ChecksumPanel::totalFormatChanges() const
{
    return m_totalFormatChanges;
}

/**
 * @brief 获取累计历史记录选择次数
 */
quint64 ChecksumPanel::totalHistorySelections() const
{
    return m_totalHistorySelections;
}

/**
 * @brief 重置所有面板统计计数器(计算次数/复制次数/算法切换/剪贴板复制/输入更新/模式切换/历史选择)
 */
void ChecksumPanel::resetPanelStatistics()
{
    m_totalCalculations = 0;
    m_totalCopyActions = 0;
    m_totalAlgorithmChanges = 0;
    m_totalCopyToClipboard = 0;
    m_totalInputUpdates = 0;
    m_totalFormatChanges = 0;
    m_totalHistorySelections = 0;
}
