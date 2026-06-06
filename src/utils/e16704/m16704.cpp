#include "e16704/m16704.h"
QVector<double> m16704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
