#include "e25704/m25704.h"
QVector<double> m25704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
