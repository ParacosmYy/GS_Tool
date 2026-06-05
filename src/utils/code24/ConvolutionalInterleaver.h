/**
 * @file ConvolutionalInterleaver.h
 * @brief 卷积交织器
 */

#pragma once

#include <QObject>
#include <QVector>

class ConvolutionalInterleaver : public QObject
{
    Q_OBJECT
public:
    explicit ConvolutionalInterleaver(QObject* parent = nullptr) : QObject(parent) {}
};
