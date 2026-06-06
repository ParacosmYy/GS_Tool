#include "m26092/m26092.h"
QVector<double> m26092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
