#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QByteArray>
#include <QVariantMap>
#include <QVector>
#include <cstring>

#include "protocol/JustFloatBridge.h"
#include "protocol/FireWaterBridge.h"

// ============================================================
// 辅助工具函数
// ============================================================

// 将多个float值打包为小端字节流 + JustFloat尾部标记
static QByteArray makeJustFloatFrame(const QVector<float>& values)
{
    QByteArray frame;
    frame.reserve(values.size() * 4 + 4);
    for (float v : values) {
        char buf[4];
        std::memcpy(buf, &v, sizeof(float));
        frame.append(buf, 4);
    }
    // 尾部标记: 00 00 80 7F
    frame.append('\x00');
    frame.append('\x00');
    frame.append('\x80');
    frame.append('\x7f');
    return frame;
}

// 将float值转为小端字节流（不含尾部标记）
static QByteArray floatToBytes(float v)
{
    char buf[4];
    std::memcpy(buf, &v, sizeof(float));
    return QByteArray(buf, 4);
}

// ============================================================
// JustFloatBridge 单元测试
// ============================================================

class TestJustFloatBridge : public QObject {
    Q_OBJECT

private slots:

    // ----------------------------------------------------------
    // 1. 单帧4通道: 4个float + 尾部标记 -> 验证4个通道值
    // ----------------------------------------------------------
    void testSingleFrameFourChannels()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        QVector<float> values = {1.0f, 2.0f, 3.0f, 4.0f};
        QByteArray frame = makeJustFloatFrame(values);
        bridge.feed(frame);

        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 4);

        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 2.0);
        QCOMPARE(fields[QStringLiteral("CH3")].toDouble(), 3.0);
        QCOMPARE(fields[QStringLiteral("CH4")].toDouble(), 4.0);

        // rawFrame应该与输入完全一致
        QByteArray rawFrame = spy.at(0).at(1).toByteArray();
        QCOMPARE(rawFrame, frame);

        // 通道数应自动检测为4
        QCOMPARE(bridge.channelCount(), 4);
    }

    // ----------------------------------------------------------
    // 2. 多帧连续: 两帧数据连续到达 -> 验证两个frameParsed信号
    // ----------------------------------------------------------
    void testMultipleConsecutiveFrames()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        QVector<float> frame1Values = {10.0f, 20.0f};
        QVector<float> frame2Values = {30.0f, 40.0f};

        QByteArray data = makeJustFloatFrame(frame1Values) + makeJustFloatFrame(frame2Values);
        bridge.feed(data);

        QCOMPARE(spy.count(), 2);

        // 第一帧
        QVariantMap fields1 = spy.at(0).at(0).toMap();
        QCOMPARE(fields1.size(), 2);
        QCOMPARE(fields1[QStringLiteral("CH1")].toDouble(), 10.0);
        QCOMPARE(fields1[QStringLiteral("CH2")].toDouble(), 20.0);

        // 第二帧
        QVariantMap fields2 = spy.at(1).at(0).toMap();
        QCOMPARE(fields2.size(), 2);
        QCOMPARE(fields2[QStringLiteral("CH1")].toDouble(), 30.0);
        QCOMPARE(fields2[QStringLiteral("CH2")].toDouble(), 40.0);

        QCOMPARE(bridge.channelCount(), 2);
    }

    // ----------------------------------------------------------
    // 3. 分片数据: 尾部标记分两次feed到达 -> 验证正确拼接
    // ----------------------------------------------------------
    void testFragmentedTailMarker()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        QVector<float> values = {5.5f, 6.6f};
        QByteArray fullFrame = makeJustFloatFrame(values);

        // 把完整帧拆为两片: float数据+尾部前两字节, 尾部后两字节
        int splitPoint = fullFrame.size() - 2;  // 最后2字节单独feed
        QByteArray part1 = fullFrame.left(splitPoint);
        QByteArray part2 = fullFrame.mid(splitPoint);

        bridge.feed(part1);
        QCOMPARE(spy.count(), 0);  // 不完整, 不应触发

        bridge.feed(part2);
        QCOMPARE(spy.count(), 1);  // 拼接后触发

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 2);
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 5.5);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 6.6);
    }

    // ----------------------------------------------------------
    // 3b. 更极端的分片: 数据逐字节feed
    // ----------------------------------------------------------
    void testByteByByteFeed()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        QVector<float> values = {1.0f};
        QByteArray frame = makeJustFloatFrame(values);

        for (int i = 0; i < frame.size(); ++i) {
            bridge.feed(QByteArray(1, frame[i]));
        }

        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
    }

    // ----------------------------------------------------------
    // 4. 自动检测通道数: 第一帧8通道 -> 后续帧保持8通道
    // ----------------------------------------------------------
    void testAutoDetectEightChannels()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 第一帧: 8通道
        QVector<float> values8;
        for (int i = 0; i < 8; ++i)
            values8.append(static_cast<float>(i + 1));

        bridge.feed(makeJustFloatFrame(values8));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelCount(), 8);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 8);
        QCOMPARE(fields[QStringLiteral("CH8")].toDouble(), 8.0);

        // 第二帧: 仍然是8通道
        QVector<float> secondFrame;
        for (int i = 0; i < 8; ++i)
            secondFrame.append(static_cast<float>(i * 10.0f));

        bridge.feed(makeJustFloatFrame(secondFrame));
        QCOMPARE(spy.count(), 2);

        QVariantMap fields2 = spy.at(1).at(0).toMap();
        QCOMPARE(fields2[QStringLiteral("CH5")].toDouble(), 40.0);
    }

    // ----------------------------------------------------------
    // 5. 缓冲区溢出: 超过4096字节无尾部 -> 数据被丢弃(不崩溃)
    // ----------------------------------------------------------
    void testBufferOverflowProtection()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 喂入5000字节无尾部标记的垃圾数据
        QByteArray junk(5000, '\xAA');
        bridge.feed(junk);

        // 不应崩溃, 不应有信号
        QCOMPARE(spy.count(), 0);

        // 之后喂入一个合法帧, 应该仍能正常解析
        // 注意: 缓冲区会丢弃最旧的数据, 但合法帧可能被保留
        QVector<float> values = {42.0f};
        QByteArray frame = makeJustFloatFrame(values);
        bridge.feed(frame);

        // 只要帧被正确拼接就应该能解析
        QCOMPARE(spy.count(), 1);
        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 42.0);
    }

    // ----------------------------------------------------------
    // 6. 空数据feed -> 无信号
    // ----------------------------------------------------------
    void testEmptyFeed()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed(QByteArray());
        QCOMPARE(spy.count(), 0);

        // 空QByteArray
        bridge.feed(QByteArray(""));
        QCOMPARE(spy.count(), 0);
    }

    // ----------------------------------------------------------
    // 7. 32通道限制: 超过32通道的帧 -> 不解析该帧
    // ----------------------------------------------------------
    void testMaxChannelLimit()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 33通道的帧 -> 超过kMaxChannels(32), 应被跳过
        QVector<float> values33;
        for (int i = 0; i < 33; ++i)
            values33.append(static_cast<float>(i));

        bridge.feed(makeJustFloatFrame(values33));
        QCOMPARE(spy.count(), 0);

        // 32通道的帧 -> 恰好在限制内, 应正常解析
        QVector<float> values32;
        for (int i = 0; i < 32; ++i)
            values32.append(static_cast<float>(i + 1));

        bridge.feed(makeJustFloatFrame(values32));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelCount(), 32);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 32);
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(fields[QStringLiteral("CH32")].toDouble(), 32.0);
    }

    // ----------------------------------------------------------
    // 8. reset() 清除状态
    // ----------------------------------------------------------
    void testReset()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);

        QVector<float> values = {1.0f, 2.0f};
        bridge.feed(makeJustFloatFrame(values));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelCount(), 2);

        bridge.reset();
        QCOMPARE(bridge.channelCount(), 0);

        // reset后重新检测通道数
        spy.clear();
        QVector<float> newValues = {10.0f, 20.0f, 30.0f};
        bridge.feed(makeJustFloatFrame(newValues));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelCount(), 3);
    }

    // ----------------------------------------------------------
    // 9. setFixedChannelCount 固定通道数
    // ----------------------------------------------------------
    void testFixedChannelCount()
    {
        JustFloatBridge bridge;
        bridge.setFixedChannelCount(3);
        QCOMPARE(bridge.channelCount(), 3);

        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 3通道帧 -> 应正常解析
        QVector<float> values = {1.0f, 2.0f, 3.0f};
        bridge.feed(makeJustFloatFrame(values));
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 3);
        QCOMPARE(fields[QStringLiteral("CH3")].toDouble(), 3.0);
    }

    // ----------------------------------------------------------
    // 10. 通道数不匹配: 第一帧2通道, 第二帧3通道 -> 跳过不匹配帧
    // ----------------------------------------------------------
    void testChannelMismatch()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 第一帧: 2通道
        bridge.feed(makeJustFloatFrame({1.0f, 2.0f}));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelCount(), 2);

        // 第二帧: 3通道(不匹配) -> 应跳过
        bridge.feed(makeJustFloatFrame({10.0f, 20.0f, 30.0f}));
        QCOMPARE(spy.count(), 1);  // 仍然是1, 新帧被跳过

        // 第三帧: 2通道(匹配) -> 正常解析
        bridge.feed(makeJustFloatFrame({100.0f, 200.0f}));
        QCOMPARE(spy.count(), 2);
    }

    // ----------------------------------------------------------
    // 11. 只有尾部标记无数据 -> 不应解析(零float数据不合法)
    // ----------------------------------------------------------
    void testTailOnlyNoData()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 仅4字节尾部标记, 无float数据
        QByteArray tailOnly;
        tailOnly.append('\x00');
        tailOnly.append('\x00');
        tailOnly.append('\x80');
        tailOnly.append('\x7f');

        bridge.feed(tailOnly);
        QCOMPARE(spy.count(), 0);
        QCOMPARE(bridge.channelCount(), 0);
    }

    // ----------------------------------------------------------
    // 12. 非对齐的float数据 + 尾部 -> 跳过不合法帧
    // ----------------------------------------------------------
    void testNonAlignedFloatData()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 5字节垃圾 + 尾部标记 = 9字节, float数据不是4的倍数
        QByteArray badFrame;
        badFrame.append('\x01');
        badFrame.append(floatToBytes(1.0f));
        badFrame.append('\x00');
        badFrame.append('\x00');
        badFrame.append('\x80');
        badFrame.append('\x7f');

        bridge.feed(badFrame);
        QCOMPARE(spy.count(), 0);
    }

    // ----------------------------------------------------------
    // 13. name() 返回正确值
    // ----------------------------------------------------------
    void testName()
    {
        JustFloatBridge bridge;
        QCOMPARE(bridge.name(), QStringLiteral("JustFloat"));
    }

    // ----------------------------------------------------------
    // 14. setFixedChannelCount 边界值
    // ----------------------------------------------------------
    void testSetFixedChannelCountBounds()
    {
        JustFloatBridge bridge;

        // 负数 -> 应clamp到0
        bridge.setFixedChannelCount(-5);
        QCOMPARE(bridge.channelCount(), 0);

        // 超过最大值 -> 应clamp到32
        bridge.setFixedChannelCount(100);
        QCOMPARE(bridge.channelCount(), 32);

        // 正常值
        bridge.setFixedChannelCount(16);
        QCOMPARE(bridge.channelCount(), 16);
    }

    // ----------------------------------------------------------
    // 15. 尾部标记出现在float数据中间 -> 应正确处理
    // ----------------------------------------------------------
    void testTailMarkerInFloatData()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 构造一个特殊场景: NaN值(00 00 80 7F)作为数据 + 真正的尾部
        // 当NaN出现在数据中, 解析器可能将其误认为尾部
        // 这是JustFloat协议的已知限制, 测试当前行为

        // 首先建立2通道
        bridge.feed(makeJustFloatFrame({1.0f, 2.0f}));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelCount(), 2);

        // 包含NaN值的帧: NaN + 1.0 + 尾部
        // NaN的小端表示就是 00 00 80 7F, 和尾部标记完全相同
        // 这会导致解析器先遇到"假尾部", 帧不对齐, 跳过
        float nanValue = std::numeric_limits<float>::quiet_NaN();
        QByteArray trickyFrame;
        trickyFrame.append(floatToBytes(nanValue));
        trickyFrame.append(floatToBytes(1.0f));
        trickyFrame.append('\x00');
        trickyFrame.append('\x00');
        trickyFrame.append('\x80');
        trickyFrame.append('\x7f');

        bridge.feed(trickyFrame);
        // 由于NaN和尾部标记相同, 解析器行为取决于实现
        // 当前实现在第一个00 00 80 7F处发现floatPayloadSize=0, 跳过
        // 然后在第二个00 00 80 7F处发现2通道, 但通道数不匹配(已有2通道 vs 检测到0通道跳过)
        // 实际行为: 第一个匹配处 floatPayloadSize=0, continue跳过
        // 第二个匹配处 floatPayloadSize=8, detectedChannels=2, 匹配 -> 解析
        QCOMPARE(spy.count(), 2);
    }

    // ----------------------------------------------------------
    // 16. 单通道
    // ----------------------------------------------------------
    void testSingleChannel()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed(makeJustFloatFrame({3.14f}));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelCount(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 1);
        QVERIFY(qFuzzyCompare(fields[QStringLiteral("CH1")].toDouble(), 3.14));
    }

    // ----------------------------------------------------------
    // 17. 负值和零值float
    // ----------------------------------------------------------
    void testNegativeAndZeroFloats()
    {
        JustFloatBridge bridge;
        QSignalSpy spy(&bridge, &JustFloatBridge::frameParsed);
        QVERIFY(spy.isValid());

        QVector<float> values = {-1.5f, 0.0f, 100.0f};
        bridge.feed(makeJustFloatFrame(values));
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), -1.5);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 0.0);
        QCOMPARE(fields[QStringLiteral("CH3")].toDouble(), 100.0);
    }
};

// ============================================================
// FireWaterBridge 单元测试
// ============================================================

class TestFireWaterBridge : public QObject {
    Q_OBJECT

private slots:

    // ----------------------------------------------------------
    // 1. 标准CSV: 两帧3通道数据
    // ----------------------------------------------------------
    void testStandardCSV()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        QByteArray data = "1.0,2.0,3.0\n4.0,5.0,6.0\n";
        bridge.feed(data);

        QCOMPARE(spy.count(), 2);

        // 第一帧: 自动命名为CH1/CH2/CH3
        QVariantMap fields1 = spy.at(0).at(0).toMap();
        QCOMPARE(fields1.size(), 3);
        QCOMPARE(fields1[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(fields1[QStringLiteral("CH2")].toDouble(), 2.0);
        QCOMPARE(fields1[QStringLiteral("CH3")].toDouble(), 3.0);

        // 第二帧
        QVariantMap fields2 = spy.at(1).at(0).toMap();
        QCOMPARE(fields2.size(), 3);
        QCOMPARE(fields2[QStringLiteral("CH1")].toDouble(), 4.0);
        QCOMPARE(fields2[QStringLiteral("CH2")].toDouble(), 5.0);
        QCOMPARE(fields2[QStringLiteral("CH3")].toDouble(), 6.0);
    }

    // ----------------------------------------------------------
    // 2. 带标题行: 第一行非数字 -> 视为通道名
    // ----------------------------------------------------------
    void testHeaderLine()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        QByteArray data = "CH1,CH2\n1.0,2.0\n";
        bridge.feed(data);

        // 头部行不发射信号, 只有数据行发射
        QCOMPARE(spy.count(), 1);

        // 验证通道名
        QStringList names = bridge.channelNames();
        QCOMPARE(names.size(), 2);
        QCOMPARE(names.at(0), QStringLiteral("CH1"));
        QCOMPARE(names.at(1), QStringLiteral("CH2"));

        // 数据行使用头部的通道名
        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 2);
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 2.0);
    }

    // ----------------------------------------------------------
    // 2b. 带中文标题行
    // ----------------------------------------------------------
    void testChineseHeaderLine()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed(QString(QStringLiteral("温度,湿度\n25.5,60.2\n")).toUtf8());

        QCOMPARE(spy.count(), 1);

        QStringList names = bridge.channelNames();
        QCOMPARE(names.size(), 2);
        QCOMPARE(names.at(0), QStringLiteral("温度"));
        QCOMPARE(names.at(1), QStringLiteral("湿度"));

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("温度")].toDouble(), 25.5);
        QCOMPARE(fields[QStringLiteral("湿度")].toDouble(), 60.2);
    }

    // ----------------------------------------------------------
    // 3. 分片行: 行数据分两次feed到达
    // ----------------------------------------------------------
    void testFragmentedLine()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 第一次feed: 不完整的行
        bridge.feed("1.0,2.0,3");
        QCOMPARE(spy.count(), 0);  // 没有换行符, 不触发

        // 第二次feed: 补全剩余部分
        bridge.feed(".0\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 3);
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 2.0);
        QCOMPARE(fields[QStringLiteral("CH3")].toDouble(), 3.0);
    }

    // ----------------------------------------------------------
    // 4. 自定义分隔符: 分号分隔
    // ----------------------------------------------------------
    void testCustomDelimiter()
    {
        FireWaterBridge bridge;
        bridge.setDelimiter(QStringLiteral(";"));
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        QByteArray data = "10.5;20.5;30.5\n";
        bridge.feed(data);

        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 3);
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 10.5);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 20.5);
        QCOMPARE(fields[QStringLiteral("CH3")].toDouble(), 30.5);
    }

    // ----------------------------------------------------------
    // 5. \r\n行尾: Windows风格的CRLF
    // ----------------------------------------------------------
    void testCRLFLineEnding()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        QByteArray data = "1.0,2.0\r\n3.0,4.0\r\n";
        bridge.feed(data);

        QCOMPARE(spy.count(), 2);

        QVariantMap fields1 = spy.at(0).at(0).toMap();
        QCOMPARE(fields1[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(fields1[QStringLiteral("CH2")].toDouble(), 2.0);

        QVariantMap fields2 = spy.at(1).at(0).toMap();
        QCOMPARE(fields2[QStringLiteral("CH1")].toDouble(), 3.0);
        QCOMPARE(fields2[QStringLiteral("CH2")].toDouble(), 4.0);
    }

    // ----------------------------------------------------------
    // 6. 空行跳过: 连续换行不产生信号
    // ----------------------------------------------------------
    void testEmptyLineSkip()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        QByteArray data = "1.0,2.0\n\n\n3.0,4.0\n";
        bridge.feed(data);

        // 空行被跳过, 只有2个数据行
        QCOMPARE(spy.count(), 2);

        QVariantMap fields1 = spy.at(0).at(0).toMap();
        QCOMPARE(fields1[QStringLiteral("CH1")].toDouble(), 1.0);

        QVariantMap fields2 = spy.at(1).at(0).toMap();
        QCOMPARE(fields2[QStringLiteral("CH1")].toDouble(), 3.0);
    }

    // ----------------------------------------------------------
    // 7. 非数字值: 包含非数字的token -> 该通道值为NaN
    // ----------------------------------------------------------
    void testNonNumericValueHandling()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 第一行建立2通道
        bridge.feed("1.0,2.0\n");
        QCOMPARE(spy.count(), 1);
        spy.clear();

        // 第二行包含非数字值"error"
        bridge.feed("3.0,error\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 2);
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 3.0);
        // 非数字值 -> NaN
        double ch2Value = fields[QStringLiteral("CH2")].toDouble();
        QVERIFY(std::isnan(ch2Value));
    }

    // ----------------------------------------------------------
    // 8. 缓冲区溢出保护: 超过8192字节的垃圾数据
    // ----------------------------------------------------------
    void testBufferOverflowProtection()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 喂入9000字节无换行符的数据
        QByteArray junk(9000, 'A');
        bridge.feed(junk);
        QCOMPARE(spy.count(), 0);  // 不崩溃

        // 之后喂入合法数据
        bridge.feed("1.0,2.0\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
    }

    // ----------------------------------------------------------
    // 9. 空数据feed -> 无信号
    // ----------------------------------------------------------
    void testEmptyFeed()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed(QByteArray());
        QCOMPARE(spy.count(), 0);
    }

    // ----------------------------------------------------------
    // 10. reset() 清除状态
    // ----------------------------------------------------------
    void testReset()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);

        // 先建立头部
        bridge.feed("Alpha,Beta\n1.0,2.0\n");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelNames().size(), 2);

        bridge.reset();
        QVERIFY(bridge.channelNames().isEmpty());

        // reset后重新检测
        spy.clear();
        bridge.feed("10.0,20.0,30.0\n");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(bridge.channelNames().size(), 3);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 10.0);
    }

    // ----------------------------------------------------------
    // 11. name() 返回正确值
    // ----------------------------------------------------------
    void testName()
    {
        FireWaterBridge bridge;
        QCOMPARE(bridge.name(), QStringLiteral("FireWater"));
    }

    // ----------------------------------------------------------
    // 12. setDelimiter 空字符串 -> 默认逗号
    // ----------------------------------------------------------
    void testSetDelimiterEmpty()
    {
        FireWaterBridge bridge;
        bridge.setDelimiter(QString());
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);

        // 空分隔符应回退到逗号
        bridge.feed("1.0,2.0\n");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toMap().size(), 2);
    }

    // ----------------------------------------------------------
    // 13. 超长行跳过: 超过2048字节的行
    // ----------------------------------------------------------
    void testOversizedLineSkip()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 构造一个超长行(>2048字节)
        QByteArray longLine(2100, 'X');
        longLine.append('\n');
        bridge.feed(longLine);
        QCOMPARE(spy.count(), 0);  // 超长行被跳过

        // 合法数据仍能正常处理
        bridge.feed("1.0,2.0\n");
        QCOMPARE(spy.count(), 1);
    }

    // ----------------------------------------------------------
    // 14. 头部行含空格 -> trimmed处理
    // ----------------------------------------------------------
    void testHeaderWithSpaces()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed(" Voltage , Current \n1.5,2.5\n");
        QCOMPARE(spy.count(), 1);

        QStringList names = bridge.channelNames();
        QCOMPARE(names.size(), 2);
        QCOMPARE(names.at(0), QStringLiteral("Voltage"));
        QCOMPARE(names.at(1), QStringLiteral("Current"));
    }

    // ----------------------------------------------------------
    // 15. 数据行值含空格 -> trimmed处理
    // ----------------------------------------------------------
    void testValuesWithSpaces()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed(" 1.0 , 2.0 \n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 2.0);
    }

    // ----------------------------------------------------------
    // 16. 32通道限制: 头部行超过32个通道
    // ----------------------------------------------------------
    void testMaxChannelLimit()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 构造33通道的头部
        QByteArray header;
        for (int i = 0; i < 33; ++i) {
            if (i > 0) header.append(',');
            header.append("CH" + QByteArray::number(i + 1));
        }
        header.append('\n');
        bridge.feed(header);

        // 通道名应被限制为32个
        QCOMPARE(bridge.channelNames().size(), 32);

        // 构造33通道的数据行
        QByteArray dataLine;
        for (int i = 0; i < 33; ++i) {
            if (i > 0) dataLine.append(',');
            dataLine.append(QByteArray::number(i + 1) + ".0");
        }
        dataLine.append('\n');
        bridge.feed(dataLine);

        QCOMPARE(spy.count(), 1);
        QVariantMap fields = spy.at(0).at(0).toMap();
        // 只解析前32个通道的值
        QCOMPARE(fields.size(), 32);
    }

    // ----------------------------------------------------------
    // 17. 头部行后数据行列数不匹配 -> 只解析通道名数量内的值
    // ----------------------------------------------------------
    void testColumnCountMismatch()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        // 头部: 2通道
        bridge.feed("A,B\n");
        QCOMPARE(spy.count(), 0);

        // 数据行: 4个值, 但只有2个通道名 -> 只取前2个
        bridge.feed("1.0,2.0,3.0,4.0\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 2);
        QCOMPARE(fields[QStringLiteral("A")].toDouble(), 1.0);
        QCOMPARE(fields[QStringLiteral("B")].toDouble(), 2.0);
        // CH3和CH4不应存在
        QVERIFY(!fields.contains(QStringLiteral("CH3")));
    }

    // ----------------------------------------------------------
    // 18. 连续多次feed单行数据
    // ----------------------------------------------------------
    void testMultipleSingleLineFeeds()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed("1.0,2.0\n");
        QCOMPARE(spy.count(), 1);

        bridge.feed("3.0,4.0\n");
        QCOMPARE(spy.count(), 2);

        bridge.feed("5.0,6.0\n");
        QCOMPARE(spy.count(), 3);

        QVariantMap f1 = spy.at(0).at(0).toMap();
        QVariantMap f2 = spy.at(1).at(0).toMap();
        QVariantMap f3 = spy.at(2).at(0).toMap();

        QCOMPARE(f1[QStringLiteral("CH1")].toDouble(), 1.0);
        QCOMPARE(f2[QStringLiteral("CH1")].toDouble(), 3.0);
        QCOMPARE(f3[QStringLiteral("CH1")].toDouble(), 5.0);
    }

    // ----------------------------------------------------------
    // 19. 科学计数法数值
    // ----------------------------------------------------------
    void testScientificNotation()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed("1.5e3,2.0E-1\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1500.0);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 0.2);
    }

    // ----------------------------------------------------------
    // 20. 负数值
    // ----------------------------------------------------------
    void testNegativeValues()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed("-1.5,-0.0,-99.99\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), -1.5);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 0.0);  // -0.0 -> 0.0
        QCOMPARE(fields[QStringLiteral("CH3")].toDouble(), -99.99);
    }

    // ----------------------------------------------------------
    // 21. rawFrame内容验证
    // ----------------------------------------------------------
    void testRawFrameContent()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed("1.0,2.0\n");
        QCOMPARE(spy.count(), 1);

        QByteArray rawFrame = spy.at(0).at(1).toByteArray();
        QCOMPARE(rawFrame, QByteArray("1.0,2.0\n"));
    }

    // ----------------------------------------------------------
    // 22. Tab分隔符
    // ----------------------------------------------------------
    void testTabDelimiter()
    {
        FireWaterBridge bridge;
        bridge.setDelimiter(QStringLiteral("\t"));
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed("10.0\t20.0\t30.0\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields.size(), 3);
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 10.0);
        QCOMPARE(fields[QStringLiteral("CH2")].toDouble(), 20.0);
        QCOMPARE(fields[QStringLiteral("CH3")].toDouble(), 30.0);
    }

    // ----------------------------------------------------------
    // 23. 仅\r\n的空行(CRLF空行)
    // ----------------------------------------------------------
    void testCRLFEmptyLine()
    {
        FireWaterBridge bridge;
        QSignalSpy spy(&bridge, &FireWaterBridge::frameParsed);
        QVERIFY(spy.isValid());

        bridge.feed("\r\n\r\n1.0,2.0\r\n");
        QCOMPARE(spy.count(), 1);

        QVariantMap fields = spy.at(0).at(0).toMap();
        QCOMPARE(fields[QStringLiteral("CH1")].toDouble(), 1.0);
    }
};

// ============================================================
// QTest主入口: 运行两个测试类
// ============================================================

int main(int argc, char* argv[])
{
    int result = 0;

    {
        TestJustFloatBridge tf;
        result |= QTest::qExec(&tf, argc, argv);
    }

    {
        TestFireWaterBridge fw;
        result |= QTest::qExec(&fw, argc, argv);
    }

    return result;
}

#include "test_protocol_bridges.moc"