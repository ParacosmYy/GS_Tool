/**
 * @file TimestampPanel.h
 * @brief 时间戳工具面板 UI
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供时间戳输入、格式选择和转换结果显示的交互面板。
 * 支持双向转换和结果复制。
 */

#ifndef TIMESTAMPPANEL_H
#define TIMESTAMPPANEL_H

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
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
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit TimestampPanel(QWidget *parent = nullptr);

private slots:
    /**
     * @brief 执行时间戳转换
     */
    void onConvert();

    /**
     * @brief 填入当前时间戳
     */
    void onNow();

    /**
     * @brief 复制结果到剪贴板
     */
    void onCopy();

private:
    /**
     * @brief 根据格式选择执行指定转换
     * @param text 输入文本
     * @param formatIndex 格式索引（0=自动,1=Unix秒,2=Unix毫秒,3=ISO日期）
     */
    void convertByFormat(const QString &text, int formatIndex);

    QLineEdit *m_timestampEdit;     ///< 时间戳输入框
    QComboBox *m_formatCombo;       ///< 格式选择下拉框
    QLabel *m_resultLabel;          ///< 结果显示标签
    QPushButton *m_convertBtn;      ///< 转换按钮
    QPushButton *m_nowBtn;          ///< 当前时间按钮
    QPushButton *m_copyBtn;         ///< 复制结果按钮
    TimestampAnalyzer m_analyzer;   ///< 转换引擎
};

#endif // TIMESTAMPPANEL_H
