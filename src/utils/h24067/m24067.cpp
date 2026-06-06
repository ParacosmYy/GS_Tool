#include "h24067/m24067.h"
QVector<double> m24067::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
