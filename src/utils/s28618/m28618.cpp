#include "s28618/m28618.h"
QVector<double> m28618::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
