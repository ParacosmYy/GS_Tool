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

private:
    QTextEdit *m_inputEdit;            ///< 输入区
    QComboBox *m_fromCombo;            ///< 源格式选择
    QComboBox *m_toCombo;              ///< 目标格式选择
    QPushButton *m_convertBtn;         ///< 转换按钮（"→"）
    QPushButton *m_swapBtn;            ///< 交换按钮
    QPushButton *m_copyBtn;            ///< 复制按钮
    QTextEdit *m_outputEdit;           ///< 输出区
    DataConverter m_converter;         ///< 转换引擎
};

#endif // CONVERTERPANEL_H
