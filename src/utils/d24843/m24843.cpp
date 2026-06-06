#include "d24843/m24843.h"
QVector<double> m24843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
