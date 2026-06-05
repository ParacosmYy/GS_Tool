/**
 * @file GameOfLife.h
 * @brief 康威生命游戏(Conway's Game of Life)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QPoint>

/**
 * @class GameOfLife
 * @brief 康威生命游戏 — 细胞自动机模拟器
 *
 * 支持经典规则(B3/S23)和自定义规则、多种预设模式、
 * 代数追踪、种群统计。
 * 适用于元胞自动机研究、涌现行为分析、算法教学等场景。
 */
class GameOfLife : public QObject
{
    Q_OBJECT

public:
    /** @brief 预设模式 */
    enum Pattern {
        Blinker,         /**< 闪烁器 */
        Glider,          /**< 滑翔机 */
        LightweightShip, /**< 轻量级飞船 */
        Pulsar,          /**< 脉冲星 */
        GosperGun,       /**< Gosper滑翔机枪 */
        Rpentomino       /**< R-五连块 */
    };
    Q_ENUM(Pattern)

    /** @brief 种群统计 */
    struct PopulationStats {
        int alive = 0;          /**< 存活细胞数 */
        int born = 0;           /**< 本代新生数 */
        int died = 0;           /**< 本代死亡数 */
        double density = 0.0;   /**< 种群密度 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSteps = 0;     /**< 总演进步数 */
        int totalReset = 0;     /**< 总重置次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit GameOfLife(int width = 50, int height = 50,
                         QObject* parent = nullptr);

    /**
     * @brief 演进一步
     * @return 种群统计
     */
    PopulationStats step();

    /**
     * @brief 演进N步
     * @param n 步数
     * @return 每步的种群统计
     */
    QVector<PopulationStats> stepN(int n);

    /**
     * @brief 设置细胞状态
     * @param x 列坐标
     * @param y 行坐标
     * @param alive 是否存活
     */
    void setCell(int x, int y, bool alive);

    /**
     * @brief 获取细胞状态
     * @param x 列坐标
     * @param y 行坐标
     * @return 是否存活
     */
    bool cell(int x, int y) const;

    /**
     * @brief 加载预设模式
     * @param pattern 模式类型
     * @param offsetX X偏移
     * @param offsetY Y偏移
     */
    void loadPattern(Pattern pattern, int offsetX = 0, int offsetY = 0);

    /**
     * @brief 随机填充
     * @param density 填充密度[0,1]
     */
    void randomize(double density = 0.3);

    /** @brief 清空网格 */
    void clear();

    /**
     * @brief 设置自定义规则
     * @param birth 出生条件(邻居数集合)
     * @param survival 存活条件(邻居数集合)
     */
    void setRules(const QVector<int>& birth, const QVector<int>& survival);

    /** @brief 获取网格宽度 */
    int width() const;

    /** @brief 获取网格高度 */
    int height() const;

    /** @brief 获取当前代数 */
    int generation() const;

    /** @brief 获取种群统计 */
    PopulationStats population() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 演进完成信号 */
    void stepped(int generation, int alive);

private:
    int countNeighbors(int x, int y) const;

    int m_width;
    int m_height;
    int m_generation;
    QVector<QVector<bool>> m_grid;
    QVector<QVector<bool>> m_nextGrid;
    QVector<int> m_birthRule;
    QVector<int> m_survivalRule;
    PopulationStats m_population;
    Stats m_stats;
    double m_timeSum;
};
