#include "s25818/m25818.h"
QVector<double> m25818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
