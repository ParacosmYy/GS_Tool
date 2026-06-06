#include "m15812/m15812.h"
QVector<double> m15812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
