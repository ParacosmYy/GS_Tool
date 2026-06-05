/**
 * @file DataTransformer.cpp
 * @brief 数据字节级转换管道引擎实现
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/transform/DataTransformer.h"

#include <QElapsedTimer>
#include <QtMath>

// ───────────────────── 辅助：反转单字节比特位 ─────────────────────

/**
 * @brief 反转单个字节的所有比特位
 * @param byte 输入字节
 * @return 比特位反转后的字节
 *
 * 使用查表法（256项预计算表），单次查表完成反转。
 */
static quint8 reverseByteBits(quint8 byte)
{
    static const quint8 table[256] = {
        0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
        0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
        0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
        0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
        0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
        0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
        0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
        0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
        0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
        0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
        0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
        0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
        0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
        0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
        0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
        0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
        0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
        0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
        0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
        0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
        0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
        0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
        0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
        0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
        0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
        0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
        0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
        0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
        0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
        0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
    };
    return table[byte];
}

// ───────────────────── 构造 / 基础 ─────────────────────

/**
 * @brief 构造数据变换管道
 * @param parent 父对象
 *
 * 初始化空步骤列表和零值统计。setObjectName 用于 QSS 和调试标识。
 */
DataTransformer::DataTransformer(QObject *parent)
    : QObject(parent)
    , m_customFn(nullptr)
{
    setObjectName(QStringLiteral("DataTransformer"));
}

// ───────────────────── 核心变换 ─────────────────────

/**
 * @brief 按顺序对所有已注册步骤执行变换
 * @param input 输入字节数据
 * @return 变换后的字节数据
 *
 * 使用 QElapsedTimer 计量耗时，更新吞吐量统计。
 * 单步出错时发射 error 信号并停止后续步骤，返回已变换的中间结果。
 */
QByteArray DataTransformer::transform(const QByteArray &input)
{
    QElapsedTimer timer;
    timer.start();

    const qint64 bytesIn = input.size();
    QByteArray current = input;

    for (int i = 0; i < m_steps.size(); ++i) {
        const TransformStep &step = m_steps.at(i);
        try {
            QByteArray result = applyStep(current, step);
            current = result;
        } catch (const std::exception &ex) {
            ++m_stats.transformErrors;
            emit error(tr("步骤 %1 (%2) 执行失败: %3")
                           .arg(i + 1)
                           .arg(step.name.isEmpty() ? QString::number(static_cast<int>(step.type))
                                                    : step.name)
                           .arg(QString::fromUtf8(ex.what())));
            break;
        } catch (...) {
            ++m_stats.transformErrors;
            emit error(tr("步骤 %1 (%2) 发生未知异常")
                           .arg(i + 1)
                           .arg(step.name.isEmpty() ? QString::number(static_cast<int>(step.type))
                                                    : step.name));
            break;
        }
    }

    // 更新统计
    ++m_stats.totalTransforms;
    m_stats.totalBytesIn += static_cast<quint64>(bytesIn);
    m_stats.totalBytesOut += static_cast<quint64>(current.size());

    const qint64 elapsedMs = timer.elapsed();
    if (elapsedMs > 0) {
        const double instantThroughput = static_cast<double>(bytesIn) / static_cast<double>(elapsedMs);
        // 指数移动平均平滑吞吐量
        if (m_stats.avgThroughput <= 0.0) {
            m_stats.avgThroughput = instantThroughput;
        } else {
            m_stats.avgThroughput = 0.8 * m_stats.avgThroughput + 0.2 * instantThroughput;
        }
    }

    emit transformed(current);
    return current;
}

// ───────────────────── 步骤管理 ─────────────────────

/**
 * @brief 向管道末尾追加一个变换步骤
 * @param type   变换类型
 * @param params 算法参数（groupSize/mask/width 等）
 *
 * 自动生成默认步骤名称（类型名 + 序号）。
 */
void DataTransformer::addStep(TransformType type, const QVariant &params)
{
    TransformStep step;
    step.type = type;
    step.params = params;

    // 自动生成步骤名称
    static const char *typeNames[] = {
        QT_TR_NOOP("ByteSwap"),
        QT_TR_NOOP("BitReverse"),
        QT_TR_NOOP("XorMask"),
        QT_TR_NOOP("Base64Encode"),
        QT_TR_NOOP("Base64Decode"),
        QT_TR_NOOP("HexEncode"),
        QT_TR_NOOP("HexDecode"),
        QT_TR_NOOP("EndianSwap"),
        QT_TR_NOOP("NullTransform"),
        QT_TR_NOOP("Custom")
    };
    const int idx = static_cast<int>(type);
    step.name = (idx >= 0 && idx < 10) ? tr(typeNames[idx])
                                       : tr("Unknown");

    m_steps.append(step);
}

/**
 * @brief 移除指定索引处的步骤
 * @param index 步骤索引（0-based）
 *
 * 索引越界时发射 error 信号并无操作。
 */
void DataTransformer::removeStep(int index)
{
    if (index < 0 || index >= m_steps.size()) {
        emit error(tr("移除步骤失败: 索引 %1 越界（共 %2 个步骤）")
                       .arg(index).arg(m_steps.size()));
        return;
    }
    m_steps.removeAt(index);
}

/** @brief 清除所有变换步骤，管道置空 */
void DataTransformer::clearSteps()
{
    m_steps.clear();
}

/**
 * @brief 设置自定义变换函数（用于 TransformType::Custom 步骤）
 * @param fn 接收 QByteArray 并返回变换结果的函数对象
 */
void DataTransformer::setCustomTransform(const std::function<QByteArray(const QByteArray &)> &fn)
{
    m_customFn = fn;
}

/**
 * @brief 获取当前步骤列表的副本
 * @return 步骤列表
 */
QList<DataTransformer::TransformStep> DataTransformer::steps() const
{
    return m_steps;
}

/**
 * @brief 获取累计统计信息
 * @return 统计结构体副本
 */
DataTransformer::Stats DataTransformer::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器归零（变换次数/字节数/错误数/吞吐量） */
void DataTransformer::resetStatistics()
{
    m_stats.totalTransforms = 0;
    m_stats.totalBytesIn = 0;
    m_stats.totalBytesOut = 0;
    m_stats.transformErrors = 0;
    m_stats.avgThroughput = 0.0;
}

// ───────────────────── 单步变换分派 ─────────────────────

/**
 * @brief 对输入数据应用单个变换步骤
 * @param input 输入字节数据
 * @param step  变换步骤描述
 * @return 变换后的字节数据
 *
 * 通过 switch 分派到各变换算法，出错时抛出 std::runtime_error。
 */
QByteArray DataTransformer::applyStep(const QByteArray &input, const TransformStep &step)
{
    switch (step.type) {

    // ── 字节组反转：按 N 字节为一组倒序排列 ──
    case TransformType::ByteSwap: {
        const int groupSize = step.params.toMap().value(QStringLiteral("groupSize"), 2).toInt();
        if (groupSize < 1) {
            throw std::runtime_error("ByteSwap groupSize 必须大于 0");
        }
        QByteArray result(input.size(), Qt::Uninitialized);
        const int len = input.size();
        for (int i = 0; i < len; i += groupSize) {
            const int remain = qMin(groupSize, len - i);
            for (int j = 0; j < remain; ++j) {
                result[i + j] = input[i + remain - 1 - j];
            }
        }
        return result;
    }

    // ── 比特位反转：每个字节内部比特序翻转 ──
    case TransformType::BitReverse: {
        QByteArray result(input.size(), Qt::Uninitialized);
        const int len = input.size();
        const auto *src = reinterpret_cast<const quint8 *>(input.constData());
        auto *dst = reinterpret_cast<quint8 *>(result.data());
        for (int i = 0; i < len; ++i) {
            dst[i] = reverseByteBits(src[i]);
        }
        return result;
    }

    // ── 异或掩码：每个字节与 mask 异或 ──
    case TransformType::XorMask: {
        const quint8 mask = static_cast<quint8>(
            step.params.toMap().value(QStringLiteral("mask"), 0xFF).toUInt());
        QByteArray result(input.size(), Qt::Uninitialized);
        const int len = input.size();
        const auto *src = reinterpret_cast<const quint8 *>(input.constData());
        auto *dst = reinterpret_cast<quint8 *>(result.data());
        for (int i = 0; i < len; ++i) {
            dst[i] = src[i] ^ mask;
        }
        return result;
    }

    // ── Base64 编码 ──
    case TransformType::Base64Encode:
        return input.toBase64();

    // ── Base64 解码 ──
    case TransformType::Base64Decode:
        return QByteArray::fromBase64(input);

    // ── 十六进制编码（输出大写，无分隔符） ──
    case TransformType::HexEncode:
        return input.toHex();

    // ── 十六进制解码 ──
    case TransformType::HexDecode:
        return QByteArray::fromHex(input);

    // ── 字节序交换：按 width 字节宽度反转 ──
    case TransformType::EndianSwap: {
        const int width = step.params.toMap().value(QStringLiteral("width"), 4).toInt();
        if (width != 2 && width != 4 && width != 8) {
            throw std::runtime_error("EndianSwap width 必须为 2、4 或 8");
        }
        if (input.size() % width != 0) {
            throw std::runtime_error("EndianSwap 输入长度必须是 width 的整数倍");
        }
        QByteArray result(input.size(), Qt::Uninitialized);
        const int len = input.size();
        const auto *src = reinterpret_cast<const quint8 *>(input.constData());
        auto *dst = reinterpret_cast<quint8 *>(result.data());
        for (int i = 0; i < len; i += width) {
            for (int j = 0; j < width; ++j) {
                dst[i + j] = src[i + width - 1 - j];
            }
        }
        return result;
    }

    // ── 空变换：直通，不做任何修改 ──
    case TransformType::NullTransform:
        return input;

    // ── 自定义变换函数 ──
    case TransformType::Custom: {
        if (!m_customFn) {
            throw std::runtime_error("未设置自定义变换函数");
        }
        return m_customFn(input);
    }
    }

    // 不应到达此处；防御性返回原始数据
    return input;
}
