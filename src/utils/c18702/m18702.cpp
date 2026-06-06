#include "c18702/m18702.h"
QVector<double> m18702::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
