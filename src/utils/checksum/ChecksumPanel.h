/**
 * @file ChecksumPanel.h
 * @brief 校验和计算面板 UI
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供数据输入（十六进制/ASCII/文件）、算法选择和结果显示的交互面板。
 */

#ifndef CHECKSUMPANEL_H
#define CHECKSUMPANEL_H

#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "utils/checksum/ChecksumCalculator.h"

/**
 * @class ChecksumPanel
 * @brief 校验和计算器 UI 面板
 */
class ChecksumPanel : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit ChecksumPanel(QWidget *parent = nullptr);

    /**
     * @brief 获取输入数据
     * @return 根据当前输入模式解析后的字节数组
     */
    QByteArray inputData() const;

    /**
     * @brief 获取当前选中的算法
     * @return 算法枚举值
     */
    ChecksumCalculator::Algorithm selectedAlgorithm() const;

    /**
     * @brief 获取计算结果
     * @return 最近一次校验和结果
     */
    quint64 result() const;

    // ---- 统计计数接口 ----

    /** @brief 获取累计计算次数（面板层面，不含内部自动调用） @return 计算次数 */
    quint64 totalCalculations() const;

    /** @brief 获取累计复制到剪贴板次数 @return 复制次数 */
    quint64 totalCopyActions() const;

    /** @brief 重置所有面板统计计数器(计算次数/复制次数) */
    void resetPanelStatistics();

signals:
    /**
     * @brief 计算完成信号
     * @param value 校验和值
     * @param algoName 算法名称
     */
    void calculated(quint64 value, const QString &algoName);

private slots:
    /**
     * @brief 执行校验和计算
     */
    void onCalculate();

    /**
     * @brief 复制结果到剪贴板
     */
    void onCopyResult();

    /**
     * @brief 清除计算历史
     */
    void onClearHistory();

    /**
     * @brief 从历史记录中选择并显示结果
     */
    void onHistoryItemSelected();

private:
    /**
     * @brief 添加一条计算结果到历史列表
     * @param value 校验和值
     * @param algoName 算法名称
     * @param inputHex 输入数据十六进制表示（截断显示）
     */
    void addHistoryEntry(quint64 value, const QString& algoName,
                         const QString& inputHex);

    QTextEdit *m_inputEdit;             ///< 数据输入区
    QComboBox *m_algoCombo;             ///< 算法选择下拉框
    QComboBox *m_inputModeCombo;        ///< 输入模式（十六进制/ASCII/文件）
    QLabel *m_resultLabel;              ///< 结果显示标签
    QPushButton *m_calcBtn;             ///< 计算按钮
    QPushButton *m_copyBtn;             ///< 复制结果按钮
    QPushButton *m_clearHistoryBtn;     ///< 清除历史按钮
    QListWidget *m_historyList;         ///< 计算历史列表
    ChecksumCalculator m_calculator;    ///< 计算引擎
    quint64 m_result = 0;               ///< 最近计算结果

    // 面板统计
    quint64 m_totalCalculations = 0;    ///< 累计计算次数
    quint64 m_totalCopyActions = 0;     ///< 累计复制到剪贴板次数

protected:
    /**
     * @brief 拖拽进入事件
     * @param event 拖拽事件
     */
    void dragEnterEvent(QDragEnterEvent* event) override;

    /**
     * @brief 拖拽移动事件
     * @param event 拖拽移动事件
     */
    void dragMoveEvent(QDragMoveEvent* event) override;

    /**
     * @brief 放下事件
     * @param event 放下事件
     */
    void dropEvent(QDropEvent* event) override;
};

#endif // CHECKSUMPANEL_H
