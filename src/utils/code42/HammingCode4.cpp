/**
 * @file HammingCode4.cpp
 * @brief 汉明码4实现 — 扩展汉明(SEC-DED)+多位纠错
 *
 * 实现标准汉明码和扩展汉明码(SEC-DED):
 * - 编码: 在数据位中插入校验位，可选扩展奇偶校验
 * - 解码: 计算校正子，检测并纠正单比特错误
 * - 扩展模式: 增加全局奇偶位实现双比特检测(SEC-DED)
 *
 * 校验位放置在 2^i 的位置(1-indexed)。
 */

#include "utils/code42/HammingCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 判断一个正整数是否为2的幂
 * @param x 待判断的数
 * @return true如果是2的幂次
 */
static bool isPowerOf2(int x)
{
    return x > 0 && (x & (x - 1)) == 0;
}

/**
 * @brief 构造函数，初始化默认(7,4)汉明码
 * @param parent 父对象指针
 */
HammingCode4::HammingCode4(QObject* parent)
    : QObject(parent)
{
    buildParityMatrix();
}

/**
 * @brief 设置汉明码参数
 * @param dataBits 数据位数 k
 * @param extendedParity 是否启用扩展奇偶校验(SEC-DED)
 */
void HammingCode4::setParameters(int dataBits, bool extendedParity)
{
    m_k = qMax(1, dataBits);
    m_extended = extendedParity;

    // 计算校验位数 r: 2^r >= k + r + 1
    m_r = 0;
    while ((1 << m_r) < m_k + m_r + 1) {
        m_r++;
    }

    // 码字长度 n = k + r
    m_n = m_k + m_r;
    if (m_extended) {
        m_n++;  ///< 扩展位增加1位
    }

    buildParityMatrix();
}

/**
 * @brief 构建校验矩阵
 *
 * 校验矩阵 H 的每一行对应一个校验位，列对应码字位。
 * H[i][j] = 1 表示第j个码字位参与第i个校验位的计算。
 */
void HammingCode4::buildParityMatrix()
{
    m_parityMatrix.resize(m_r);
    for (int i = 0; i < m_r; ++i) {
        m_parityMatrix[i].resize(m_n);
        for (int j = 0; j < m_n; ++j) {
            // 第i个校验位覆盖所有第(j+1)的二进制表示中第i位为1的位置
            if ((j + 1) & (1 << i)) {
                m_parityMatrix[i][j] = 1;
            } else {
                m_parityMatrix[i][j] = 0;
            }
        }
    }
}

/**
 * @brief 对数据位进行汉明编码
 *
 * 编码过程:
 * 1. 将数据位插入非校验位位置
 * 2. 计算每个校验位(基于其覆盖的所有数据位)
 * 3. 如果是扩展模式，计算全局奇偶位
 *
 * @param data 输入数据位向量(值为0或1)
 * @return 编码后的码字
 */
QVector<int> HammingCode4::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeWord(m_n, 0);

    // 步骤1: 将数据位放置在非2的幂次位置
    int dataIdx = 0;
    for (int i = 0; i < m_n - (m_extended ? 1 : 0); ++i) {
        if (!isPowerOf2(i + 1)) {
            // i+1 不是2的幂，放置数据位
            if (dataIdx < data.size() && dataIdx < m_k) {
                codeWord[i] = data[dataIdx];
            }
            dataIdx++;
        }
    }

    // 步骤2: 计算校验位
    for (int r = 0; r < m_r; ++r) {
        int parityPos = (1 << r) - 1;  ///< 0-indexed校验位位置
        int parity = 0;
        for (int j = 0; j < m_n - (m_extended ? 1 : 0); ++j) {
            if (j == parityPos) continue;  ///< 跳过校验位自身
            if (m_parityMatrix[r][j]) {
                parity ^= codeWord[j];
            }
        }
        codeWord[parityPos] = parity;
    }

    // 步骤3: 扩展奇偶位(覆盖全部位)
    if (m_extended) {
        int overallParity = 0;
        for (int i = 0; i < m_n - 1; ++i) {
            overallParity ^= codeWord[i];
        }
        codeWord[m_n - 1] = overallParity;
    }

    // 更新统计信息
    m_stats.totalEncodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(m_k, m_n);
    return codeWord;
}

/**
 * @brief 解码接收到的码字，检测并纠正错误
 *
 * 解码过程:
 * 1. 计算校正子
 * 2. 检查扩展奇偶位
 * 3. 若校正子非零且奇偶位有错 -> 单比特错误，纠正
 * 4. 若校正子非零但奇偶位无错 -> 双比特错误(不可纠正)
 *
 * @param received 接收到的码字
 * @return 纠正后的数据位
 */
QVector<int> HammingCode4::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeWord = received;
    // 补齐或截断到码字长度
    codeWord.resize(m_n);

    int errorsDetected = 0;
    int errorsCorrected = 0;

    // 计算校正子
    QVector<int> synd = syndrome(codeWord);
    int syndValue = 0;
    for (int i = 0; i < synd.size(); ++i) {
        syndValue |= (synd[i] << i);
    }

    if (m_extended && codeWord.size() >= m_n) {
        // 计算扩展奇偶位
        int overallParity = 0;
        for (int i = 0; i < m_n; ++i) {
            overallParity ^= codeWord[i];
        }

        if (syndValue != 0 && overallParity != 0) {
            // 单比特错误: 校正子指向错误位置
            int errorPos = syndValue - 1;  ///< 转为0-indexed
            if (errorPos >= 0 && errorPos < m_n - 1) {
                codeWord[errorPos] ^= 1;
                errorsDetected = 1;
                errorsCorrected = 1;
            }
        } else if (syndValue != 0 && overallParity == 0) {
            // 双比特错误(不可纠正)
            errorsDetected = 2;
            errorsCorrected = 0;
        }
        // syndValue == 0 且 overallParity == 0: 无错误
    } else if (syndValue != 0) {
        // 非扩展模式: 直接用校正子定位
        int errorPos = syndValue - 1;
        if (errorPos >= 0 && errorPos < m_n) {
            codeWord[errorPos] ^= 1;
            errorsDetected = 1;
            errorsCorrected = 1;
        }
    }

    // 提取数据位
    QVector<int> dataBits;
    for (int i = 0; i < m_n - (m_extended ? 1 : 0); ++i) {
        if (!isPowerOf2(i + 1)) {
            dataBits.append(codeWord[i]);
        }
    }

    // 更新统计信息
    m_stats.totalDecodes++;
    m_stats.totalErrorsCorrected += errorsCorrected;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(errorsDetected, errorsCorrected);
    return dataBits;
}

/**
 * @brief 检测接收码字中的错误数
 * @param received 接收到的码字
 * @return 检测到的错误数(0/1/2)
 */
int HammingCode4::detectErrors(const QVector<int>& received) const
{
    QVector<int> synd = syndrome(received);
    int syndValue = 0;
    for (int i = 0; i < synd.size(); ++i) {
        syndValue |= (synd[i] << i);
    }

    if (syndValue == 0) return 0;

    if (m_extended && received.size() >= m_n) {
        int overallParity = 0;
        for (int i = 0; i < m_n; ++i) {
            overallParity ^= received[i];
        }
        return (overallParity != 0) ? 1 : 2;
    }

    return 1;
}

/**
 * @brief 计算校正子
 *
 * 校正子 = H * r^T，其中H是校验矩阵，r是接收码字。
 * 非零校正子指示错误位置。
 *
 * @param received 接收到的码字
 * @return 校正子向量
 */
QVector<int> HammingCode4::syndrome(const QVector<int>& received) const
{
    QVector<int> synd(m_r, 0);
    int len = qMin(received.size(), m_n - (m_extended ? 1 : 0));

    for (int i = 0; i < m_r; ++i) {
        int s = 0;
        for (int j = 0; j < len; ++j) {
            if (j < received.size()) {
                s ^= (received[j] & m_parityMatrix[i][j]);
            }
        }
        synd[i] = s;
    }
    return synd;
}

/**
 * @brief 重置所有统计信息
 */
void HammingCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

