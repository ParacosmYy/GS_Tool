/**
 * @file TimestampPanel.h
 * @brief 时间戳工具面板 UI — 提供时间戳输入、格式选择和转换结果显示的交互面板
 *
 * 支持 Unix秒/Unix毫秒/ISO日期 三种格式的双向转换。
 * 包含转换历史记录列表，限制最近30条记录。
 * 会话统计: 转换次数/复制次数/分析次数/格式切换/解析次数/当前时间按钮/历史恢复
 *
 * 协作: TimestampAnalyzer(转换引擎) / TimestampPanelConvert(槽函数+统计)
 *
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#ifndef TIMESTAMPPANEL_H
#define TIMESTAMPPANEL_H

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

#include "utils/timestamp/TimestampAnalyzer.h"

/**
 * @class TimestampPanel
 * @brief 时间戳转换工具 UI 面板
 */
class TimestampPanel : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造时间戳工具面板 @param parent 父控件 */
    explicit TimestampPanel(QWidget *parent = nullptr);

private slots:
    /** @brief 执行时间戳转换 */
    void onConvert();

    /** @brief 填入当前时间戳 */
    void onNow();

    /** @brief 复制结果到剪贴板 */
    void onCopy();

    /** @brief 清除转换历史 */
    void onClearHistory();

    /** @brief 从历史记录选择恢复 */
    void onHistorySelected();

private:
    /** @brief 根据格式选择执行指定转换 @param text 输入文本 @param formatIndex 格式索引(0=自动,1=Unix秒,2=Unix毫秒,3=ISO日期) */
    void convertByFormat(const QString &text, int formatIndex);

    QLineEdit *m_timestampEdit;     ///< 时间戳输入框
    QComboBox *m_formatCombo;       ///< 格式选择下拉框
    QLabel *m_resultLabel;          ///< 结果显示标签
    QPushButton *m_convertBtn;      ///< 转换按钮
    QPushButton *m_nowBtn;          ///< 当前时间按钮
    QPushButton *m_copyBtn;         ///< 复制结果按钮
    QPushButton *m_clearHistoryBtn; ///< 清除历史按钮
    QListWidget *m_historyList;     ///< 转换历史列表
    TimestampAnalyzer m_analyzer;   ///< 转换引擎

    // ---- 统计计数器 ----
    quint64 m_totalConversions = 0; ///< 累计转换次数
    quint64 m_totalCopyActions = 0; ///< 累计复制操作次数
    quint64 m_totalAnalyses = 0;    ///< 累计时间戳分析次数
    quint64 m_totalFormatChanges = 0; ///< 累计格式切换次数
    quint64 m_totalTimestampParses = 0; ///< 累计时间戳解析次数
    quint64 m_totalFormatsSelected = 0; ///< 累计格式选中次数(下拉框选择)
    quint64 m_totalCopies = 0;       ///< 累计复制结果到剪贴板次数
    quint64 m_totalNowActions = 0;   ///< 累计"当前时间"按钮点击次数
    quint64 m_totalHistoryRestores = 0; ///< 累计历史记录恢复次数
public:
    /** @brief 获取累计转换次数 @return 转换次数 */
    quint64 totalConversions() const { return m_totalConversions; }
    /** @brief 获取累计复制操作次数 @return 复制次数 */
    quint64 totalCopyActions() const { return m_totalCopyActions; }
    /** @brief 获取累计时间戳分析次数 @return 分析次数 */
    quint64 totalAnalyses() const { return m_totalAnalyses; }
    /** @brief 获取累计格式切换次数 @return 切换次数 */
    quint64 totalFormatChanges() const { return m_totalFormatChanges; }
    /** @brief 获取累计时间戳解析次数 @return 解析次数 */
    quint64 totalTimestampParses() const { return m_totalTimestampParses; }
    /** @brief 获取累计格式选中次数 @return 格式选中次数 */
    quint64 totalFormatsSelected() const { return m_totalFormatsSelected; }
    /** @brief 获取累计复制结果次数 @return 复制次数 */
    quint64 totalCopies() const { return m_totalCopies; }
    /** @brief 获取累计"当前时间"按钮点击次数 @return 点击次数 */
    quint64 totalNowActions() const { return m_totalNowActions; }
    /** @brief 获取累计历史记录恢复次数 @return 恢复次数 */
    quint64 totalHistoryRestores() const { return m_totalHistoryRestores; }
    /** @brief 重置时间戳面板统计计数器(转换次数/复制次数/分析次数/格式切换/解析次数/当前时间/历史恢复) */
    void resetTimestampPanelStatistics();
};

#endif // TIMESTAMPPANEL_H
