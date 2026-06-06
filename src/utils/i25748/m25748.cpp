#include "i25748/m25748.h"
QVector<double> m25748::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
