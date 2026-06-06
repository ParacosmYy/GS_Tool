#include "m15232/m15232.h"
QVector<double> m15232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
