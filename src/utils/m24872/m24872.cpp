#include "m24872/m24872.h"
QVector<double> m24872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
