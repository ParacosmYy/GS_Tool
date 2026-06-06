#include "p25215/m25215.h"
QVector<double> m25215::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
