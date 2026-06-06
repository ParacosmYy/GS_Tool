#include "a28020/m28020.h"
QVector<double> m28020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
