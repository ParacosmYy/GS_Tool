#include "l8311/m8311.h"
QVector<double> m8311::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
