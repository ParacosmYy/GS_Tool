#include "a25900/m25900.h"
QVector<double> m25900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
