#include "m17812/m17812.h"
QVector<double> m17812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
