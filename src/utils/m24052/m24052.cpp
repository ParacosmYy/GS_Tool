#include "m24052/m24052.h"
QVector<double> m24052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
