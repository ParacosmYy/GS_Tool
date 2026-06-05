/**
 * @file HierarchicalDensity.h
 * @brief 层次密度聚类 — HDBSCAN风格密度聚类
 */

#pragma once

#include <QObject>
#include <QVector>

class HierarchicalDensity : public QObject
{
    Q_OBJECT
public:
    explicit HierarchicalDensity(QObject* parent = nullptr) : QObject(parent) {}
};
