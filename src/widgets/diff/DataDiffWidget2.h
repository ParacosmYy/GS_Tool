/**
 * @file DataDiffWidget2.h
 * @brief 数据对比组件 - 并排显示两组二进制数据的逐行差异
 *
 * 职责:
 *   1. 将左右两组二进制数据按行对齐显示
 *   2. 逐行比较并标记差异字节
 *   3. 支持自定义每行显示字节数
 *   4. 自定义绘制diff视图（颜色标记差异行）
 */

#pragma once
#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QList>

/**
 * @brief 数据对比组件
 *
 * 接收左右两组二进制数据，按指定字节数分行对齐后逐行比较，
 * 差异行通过paintEvent以不同颜色高亮显示。
 */
class DataDiffWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief 单行差异描述结构体 */
    struct DiffLine {
        int leftLine;            ///< 左侧行号
        int rightLine;           ///< 右侧行号
        QByteArray leftData;     ///< 左侧本行数据
        QByteArray rightData;    ///< 右侧本行数据
        bool different;          ///< 本行是否存在差异
    };

    /**
     * @brief 构造数据对比组件
     * @param parent 父widget
     */
    explicit DataDiffWidget(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~DataDiffWidget() override;

    /**
     * @brief 设置左侧数据
     * @param data 左侧二进制数据
     */
    void setLeftData(const QByteArray &data);

    /**
     * @brief 设置右侧数据
     * @param data 右侧二进制数据
     */
    void setRightData(const QByteArray &data);

    /**
     * @brief 设置每行显示的字节数
     * @param bytes 每行字节数，默认16
     */
    void setBytesPerLine(int bytes);

    /**
     * @brief 计算并返回逐行差异结果
     * @return 差异行列表
     */
    QList<DiffLine> computeDiff() const;

    /** @brief 清空左右数据和差异结果 */
    void clear();

    /**
     * @brief 获取差异行数量
     * @return 存在差异的行数
     */
    int diffCount() const;

signals:
    /**
     * @brief 差异计算完成
     * @param diffLines 差异行数
     * @param totalLines 总行数
     */
    void diffComputed(int diffLines, int totalLines);

protected:
    /** @brief 自定义绘制：左侧/右侧数据并排，差异行高亮 */
    void paintEvent(QPaintEvent *event) override;

private:
    QByteArray m_left;              ///< 左侧二进制数据
    QByteArray m_right;             ///< 右侧二进制数据
    int m_bytesPerLine = 16;        ///< 每行显示字节数
    int m_diffCount = 0;            ///< 差异行计数
};
