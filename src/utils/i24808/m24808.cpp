#include "i24808/m24808.h"
QVector<double> m24808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
