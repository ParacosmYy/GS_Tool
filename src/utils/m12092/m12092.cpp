#include "m12092/m12092.h"
QVector<double> m12092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
