#include "i24868/m24868.h"
QVector<double> m24868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
