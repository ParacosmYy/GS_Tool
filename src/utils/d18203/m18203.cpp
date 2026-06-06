#include "d18203/m18203.h"
QVector<double> m18203::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
