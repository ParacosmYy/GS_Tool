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

    /** @brief 获取累计转换操作次数 */
    quint64 totalConversions() const { return m_totalConversions; }

    /** @brief 获取累计复制操作次数 */
    quint64 totalCopyActions() const { return m_totalCopyActions; }

    /** @brief 重置所有统计计数器 */
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
};

#endif // CONVERTERPANEL_H
