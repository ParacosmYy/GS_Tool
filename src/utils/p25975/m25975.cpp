#include "p25975/m25975.h"
QVector<double> m25975::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
