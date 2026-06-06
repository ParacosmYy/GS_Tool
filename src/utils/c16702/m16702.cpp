#include "c16702/m16702.h"
QVector<double> m16702::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
