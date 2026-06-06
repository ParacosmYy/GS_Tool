#include "b25881/m25881.h"
QVector<double> m25881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
