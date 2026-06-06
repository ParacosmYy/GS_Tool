#include "i25928/m25928.h"
QVector<double> m25928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
