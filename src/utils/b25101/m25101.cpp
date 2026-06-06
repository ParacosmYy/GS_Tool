#include "b25101/m25101.h"
QVector<double> m25101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
