#include "c9702/m9702.h"
QVector<double> m9702::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
