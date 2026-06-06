#include "c32702/m32702.h"
QVector<double> m32702::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
