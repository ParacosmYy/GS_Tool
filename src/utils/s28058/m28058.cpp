#include "s28058/m28058.h"
QVector<double> m28058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
