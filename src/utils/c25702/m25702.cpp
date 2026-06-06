#include "c25702/m25702.h"
QVector<double> m25702::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
