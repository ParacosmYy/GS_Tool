/**
 * @file ConverterPanel.h
 * @brief 数据格式转换面板 UI
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供格式选择、输入输出区域的交互面板。
 */

#ifndef CONVERTERPANEL_H
#define CONVERTERPANEL_H

#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
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

private:
    QTextEdit *m_inputEdit;            ///< 输入区
    QComboBox *m_fromCombo;            ///< 源格式选择
    QComboBox *m_toCombo;              ///< 目标格式选择
    QPushButton *m_convertBtn;         ///< 转换按钮
    QTextEdit *m_outputEdit;           ///< 输出区
    DataConverter m_converter;         ///< 转换引擎
};

#endif // CONVERTERPANEL_H
