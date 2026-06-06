#include "s18558/m18558.h"
QVector<double> m18558::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
