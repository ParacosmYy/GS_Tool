#include "c35702/m35702.h"
QVector<double> m35702::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
