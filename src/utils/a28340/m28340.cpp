#include "a28340/m28340.h"
QVector<double> m28340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
