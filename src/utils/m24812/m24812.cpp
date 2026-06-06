#include "m24812/m24812.h"
QVector<double> m24812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
