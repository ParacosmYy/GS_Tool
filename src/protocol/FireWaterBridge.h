#ifndef FIREWATERBRIDGE_H
#define FIREWATERBRIDGE_H

#include "protocol/IProtocolBridge.h"

#include <QByteArray>
#include <QVariantMap>
#include <QString>
#include <QStringList>

// FireWater协议桥 -- 解析VOFA+兼容的FireWater CSV字节流
//
// 协议格式:
//   行1（可选）: 通道名称，逗号分隔，以\n结尾
//     例: "温度,湿度,压力\n"
//   行2+: 数据值，逗号分隔，以\n结尾
//     例: "25.5,60.2,1013.25\n"
//
// 自动检测逻辑:
//   - 如果第一行的第一个token不是有效数字，则视为通道名称行
//   - 如果第一行就是数据，则自动命名为 "CH1", "CH2", ...
//
// 数据层: 不依赖任何表现层类
class FireWaterBridge : public IProtocolBridge {
    Q_OBJECT

public:
    explicit FireWaterBridge(QObject* parent = nullptr);

    // IProtocolBridge接口实现
    void feed(const QByteArray& data) override;
    void reset() override;
    QString name() const override;

    // 设置CSV分隔符（默认为逗号','）
    void setDelimiter(const QString& delimiter);

    // 获取当前通道名称列表（头部检测后生效）
    QStringList channelNames() const;

private:
    // 尝试从缓冲区中提取并解析所有完整的行
    void parseLines();

    // 解析单行数据
    // line: 一行文本（不含\n结尾）
    void processLine(const QString& line);

    // 将CSV值行解析为QVariantMap（通道名→值）
    // 返回空Map如果解析失败
    QVariantMap parseValueLine(const QString& line) const;

    QByteArray m_buffer;            // 累积的原始字节缓冲区

    QStringList m_channelNames;     // 通道名称列表
    bool m_headerReceived;          // 是否已接收到头部行
    bool m_firstLineIsData;         // 第一行是否为数据行（无头部）

    QString m_delimiter;            // CSV分隔符（默认逗号）

    static constexpr int kMaxBufferSize = 8192;  // 最大缓冲区保护
    static constexpr int kMaxLineSize = 2048;    // 单行最大长度
    static constexpr int kMaxChannels = 32;      // 最大通道数保护
};

#endif // FIREWATERBRIDGE_H
