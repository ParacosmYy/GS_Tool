#include "h25827/m25827.h"
QVector<double> m25827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
