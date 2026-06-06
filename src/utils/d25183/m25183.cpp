#include "d25183/m25183.h"
QVector<double> m25183::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
