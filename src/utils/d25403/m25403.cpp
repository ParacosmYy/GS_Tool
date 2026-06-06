#include "d25403/m25403.h"
QVector<double> m25403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
