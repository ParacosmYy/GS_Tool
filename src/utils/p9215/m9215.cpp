#include "p9215/m9215.h"
QVector<double> m9215::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
