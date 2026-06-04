/** @file ChecksumPanel.h @brief 校验和计算面板 UI -- 数据输入(十六进制/ASCII/文件)、算法选择、结果显示 */
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

/** @brief 校验和计算器 UI 面板 */
class ChecksumPanel : public QWidget {
    Q_OBJECT

public:
    explicit ChecksumPanel(QWidget *parent = nullptr);
    QByteArray inputData() const;                              ///< 获取输入数据
    ChecksumCalculator::Algorithm selectedAlgorithm() const;   ///< 获取当前算法
    quint64 result() const;                                    ///< 获取计算结果

    // ---- 统计计数接口 ----
    quint64 totalCalculations() const;       ///< 累计计算次数
    quint64 totalCopyActions() const;        ///< 累计复制结果次数
    quint64 totalAlgorithmChanges() const;   ///< 累计算法切换次数
    quint64 totalCopyToClipboard() const;    ///< 累计剪贴板复制次数
    quint64 totalInputUpdates() const;       ///< 累计输入更新次数
    quint64 totalFormatChanges() const;      ///< 累计输入模式切换次数
    quint64 totalHistorySelections() const;  ///< 累计历史选择次数
    void resetPanelStatistics();             ///< 重置所有面板统计

signals:
    void calculated(quint64 value, const QString &algoName); ///< 计算完成信号

private slots:
    void onCalculate();
    void onCopyResult();
    void onClearHistory();
    void onHistoryItemSelected();

private:
    void addHistoryEntry(quint64 value, const QString& algoName, const QString& inputHex);

    QTextEdit *m_inputEdit;
    QComboBox *m_algoCombo, *m_inputModeCombo;
    QLabel *m_resultLabel;
    QPushButton *m_calcBtn, *m_copyBtn, *m_clearHistoryBtn;
    QListWidget *m_historyList;
    ChecksumCalculator m_calculator;
    quint64 m_result = 0;
    quint64 m_totalCalculations = 0, m_totalCopyActions = 0, m_totalAlgorithmChanges = 0;
    quint64 m_totalCopyToClipboard = 0, m_totalInputUpdates = 0, m_totalFormatChanges = 0;
    quint64 m_totalHistorySelections = 0;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};

#endif // CHECKSUMPANEL_H
