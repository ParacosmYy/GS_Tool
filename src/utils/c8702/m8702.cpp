#include "c8702/m8702.h"
QVector<double> m8702::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
