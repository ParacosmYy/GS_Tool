#include "i24408/m24408.h"
QVector<double> m24408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
