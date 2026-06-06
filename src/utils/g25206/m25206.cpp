#include "g25206/m25206.h"
QVector<double> m25206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
