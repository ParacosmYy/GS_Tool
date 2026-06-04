/**
 * @file ConverterPanel.h
 * @brief 数据格式转换面板 UI
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供格式选择、输入输出区域、交换和复制功能的交互面板。
 */

#ifndef CONVERTERPANEL_H
#define CONVERTERPANEL_H

#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include "utils/converter/DataConverter.h"

/**
 * @class ConverterPanel
 * @brief 数据格式转换器 UI 面板
 */
class ConverterPanel : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit ConverterPanel(QWidget *parent = nullptr);

    /**
     * @brief 设置输入内容
     * @param text 输入文本
     */
    void setInput(const QString &text);

    /**
     * @brief 获取输出内容
     * @return 转换后的文本
     */
    QString output() const;

    /** @brief 获取累计转换操作次数 @return 转换次数 */
    quint64 totalConversions() const { return m_totalConversions; }

    /** @brief 获取累计复制操作次数 @return 复制次数 */
    quint64 totalCopyActions() const { return m_totalCopyActions; }

    /** @brief 获取累计格式交换次数 @return 交换次数 */
    quint64 totalFormatSwaps() const { return m_totalFormatSwaps; }

    /** @brief 获取累计输入内容变更次数 @return 输入变更次数 */
    quint64 totalInputChanges() const { return m_totalInputChanges; }

    /** @brief 获取累计转换错误次数 @return 错误次数 */
    quint64 totalErrors() const { return m_totalErrors; }

    /** @brief 获取累计格式下拉框切换次数(源/目标格式变更) @return 格式切换总次数 */
    quint64 totalFormatSwitches() const { return m_totalFormatSwitches; }

    /** @brief 获取累计剪贴板操作次数(复制到剪贴板) @return 剪贴板操作总次数 */
    quint64 totalClipboardOps() const { return m_totalClipboardOps; }

    /** @brief 重置所有转换器统计计数器(转换次数/复制次数/格式交换/输入变更/错误/格式切换/剪贴板) */
    void resetStatistics();

private slots:
    /**
     * @brief 执行格式转换
     */
    void onConvert();

    /**
     * @brief 交换源/目标格式
     */
    void onSwap();

    /**
     * @brief 复制输出到剪贴板
     */
    void onCopy();

    /**
     * @brief 清除转换历史
     */
    void onClearHistory();

    /**
     * @brief 从历史选择恢复
     */
    void onHistorySelected();

private:
    QTextEdit *m_inputEdit;            ///< 输入区
    QComboBox *m_fromCombo;            ///< 源格式选择
    QComboBox *m_toCombo;              ///< 目标格式选择
    QPushButton *m_convertBtn;         ///< 转换按钮（"→"）
    QPushButton *m_swapBtn;            ///< 交换按钮
    QPushButton *m_copyBtn;            ///< 复制按钮
    QTextEdit *m_outputEdit;           ///< 输出区
    QPushButton *m_clearHistoryBtn;    ///< 清除历史按钮
    QListWidget *m_historyList;        ///< 转换历史列表
    DataConverter m_converter;         ///< 转换引擎

    // ---- 统计计数器 ----
    quint64 m_totalConversions = 0;    ///< 累计转换操作次数
    quint64 m_totalCopyActions = 0;    ///< 累计复制操作次数
    quint64 m_totalFormatSwaps = 0;    ///< 累计格式交换次数
    quint64 m_totalInputChanges = 0;   ///< 累计输入内容变更次数
    quint64 m_totalErrors = 0;         ///< 累计转换错误次数
    quint64 m_totalFormatSwitches = 0; ///< 累计格式下拉框切换次数
    quint64 m_totalClipboardOps = 0;   ///< 累计剪贴板操作次数
};

#endif // CONVERTERPANEL_H
