#include "d15403/m15403.h"
QVector<double> m15403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
