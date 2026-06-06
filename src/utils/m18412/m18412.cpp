#include "m18412/m18412.h"
QVector<double> m18412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
