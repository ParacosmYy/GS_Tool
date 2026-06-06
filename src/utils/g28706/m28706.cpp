#include "g28706/m28706.h"
QVector<double> m28706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
