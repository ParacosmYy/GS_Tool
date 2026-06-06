#include "p25095/m25095.h"
QVector<double> m25095::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
