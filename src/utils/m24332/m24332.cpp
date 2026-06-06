#include "m24332/m24332.h"
QVector<double> m24332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
