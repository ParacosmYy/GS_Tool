/**
 * @file ChecksumPanel.cpp
 * @brief 校验和计算面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/checksum/ChecksumPanel.h"

/**
 * @brief 构造函数，初始化面板布局
 */
ChecksumPanel::ChecksumPanel(QWidget *parent)
    : QWidget(parent)
    , m_inputEdit(new QTextEdit(this))
    , m_algoCombo(new QComboBox(this))
    , m_resultLabel(new QLabel(tr("结果：-"), this))
    , m_calcBtn(new QPushButton(tr("计算"), this))
{
    setObjectName(QStringLiteral("ChecksumPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 算法选择行
    auto *algoLayout = new QHBoxLayout();
    algoLayout->addWidget(new QLabel(tr("算法："), this));

    // 填充算法列表
    const auto algorithms = {
        ChecksumCalculator::CRC8, ChecksumCalculator::CRC16Ccitt,
        ChecksumCalculator::CRC16Modbus, ChecksumCalculator::CRC16Kermit,
        ChecksumCalculator::CRC32, ChecksumCalculator::CRC32C,
        ChecksumCalculator::Xor8, ChecksumCalculator::Sum8,
        ChecksumCalculator::Sum16, ChecksumCalculator::Sum32,
        ChecksumCalculator::CustomCrc
    };
    for (auto alg : algorithms) {
        m_algoCombo->addItem(ChecksumCalculator::algorithmName(alg),
                             static_cast<int>(alg));
    }
    algoLayout->addWidget(m_algoCombo);
    algoLayout->addStretch();
    algoLayout->addWidget(m_calcBtn);

    // 输入区
    m_inputEdit->setPlaceholderText(tr("输入十六进制数据（如：01 02 FF）..."));
    m_inputEdit->setMaximumHeight(100);

    // 结果标签
    m_resultLabel->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14pt;"));

    mainLayout->addLayout(algoLayout);
    mainLayout->addWidget(m_inputEdit);
    mainLayout->addWidget(m_resultLabel);

    // 连接信号
    connect(m_calcBtn, &QPushButton::clicked,
            this, &ChecksumPanel::onCalculate);
}

/**
 * @brief 解析输入的十六进制文本为字节数组
 */
QByteArray ChecksumPanel::inputData() const
{
    QString text = m_inputEdit->toPlainText().simplified();
    text.remove(' ');
    return QByteArray::fromHex(text.toUtf8());
}

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

/**
 * @brief 执行校验和计算
 */
void ChecksumPanel::onCalculate()
{
    QByteArray data = inputData();
    if (data.isEmpty()) {
        m_resultLabel->setText(tr("结果：无数据"));
        return;
    }

    auto alg = selectedAlgorithm();
    m_result = m_calculator.calculate(data, alg);
    QString name = ChecksumCalculator::algorithmName(alg);

    m_resultLabel->setText(tr("结果：%1 (%2)")
                               .arg(m_result, 0, 16, QLatin1Char('0'))
                               .arg(name));

    emit calculated(m_result, name);
}
