#include "h25607/m25607.h"
QVector<double> m25607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
