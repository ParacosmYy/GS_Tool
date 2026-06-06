#include "i24108/m24108.h"
QVector<double> m24108::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
