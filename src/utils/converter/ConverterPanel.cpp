/**
 * @file ConverterPanel.cpp
 * @brief 数据格式转换面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/converter/ConverterPanel.h"
#include <QLabel>
#include <QHBoxLayout>

/**
 * @brief 构造函数，初始化转换面板布局
 */
ConverterPanel::ConverterPanel(QWidget *parent)
    : QWidget(parent)
    , m_inputEdit(new QTextEdit(this))
    , m_fromCombo(new QComboBox(this))
    , m_toCombo(new QComboBox(this))
    , m_convertBtn(new QPushButton(tr("转换"), this))
    , m_outputEdit(new QTextEdit(this))
{
    setObjectName(QStringLiteral("ConverterPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 格式选择行
    auto *formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel(tr("源格式："), this));

    const auto formats = {
        DataConverter::Hex, DataConverter::Ascii, DataConverter::Base64,
        DataConverter::UrlEncode, DataConverter::Binary,
        DataConverter::Decimal, DataConverter::Octal
    };
    for (auto fmt : formats) {
        QString name = DataConverter::formatName(fmt);
        m_fromCombo->addItem(name, static_cast<int>(fmt));
        m_toCombo->addItem(name, static_cast<int>(fmt));
    }

    formatLayout->addWidget(m_fromCombo);
    formatLayout->addWidget(new QLabel(tr("目标格式："), this));
    formatLayout->addWidget(m_toCombo);
    formatLayout->addWidget(m_convertBtn);

    // 默认选择：Hex → ASCII
    m_fromCombo->setCurrentIndex(0);
    m_toCombo->setCurrentIndex(1);

    // 输入输出区
    m_inputEdit->setPlaceholderText(tr("输入待转换数据..."));
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setPlaceholderText(tr("转换结果将显示在此..."));

    mainLayout->addLayout(formatLayout);
    mainLayout->addWidget(m_inputEdit);
    mainLayout->addWidget(m_outputEdit);

    connect(m_convertBtn, &QPushButton::clicked,
            this, &ConverterPanel::onConvert);
}

/**
 * @brief 设置输入内容
 */
void ConverterPanel::setInput(const QString &text)
{
    m_inputEdit->setPlainText(text);
}

/**
 * @brief 获取输出内容
 */
QString ConverterPanel::output() const
{
    return m_outputEdit->toPlainText();
}

/**
 * @brief 执行格式转换
 */
void ConverterPanel::onConvert()
{
    QByteArray input = m_inputEdit->toPlainText().toUtf8();
    if (input.isEmpty()) {
        m_outputEdit->clear();
        return;
    }

    auto from = static_cast<DataConverter::Format>(m_fromCombo->currentData().toInt());
    auto to = static_cast<DataConverter::Format>(m_toCombo->currentData().toInt());

    QByteArray result = m_converter.convert(input, from, to);
    m_outputEdit->setPlainText(QString::fromUtf8(result));
}
