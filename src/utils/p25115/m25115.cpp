#include "p25115/m25115.h"
QVector<double> m25115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
