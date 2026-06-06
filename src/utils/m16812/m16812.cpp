#include "m16812/m16812.h"
QVector<double> m16812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
