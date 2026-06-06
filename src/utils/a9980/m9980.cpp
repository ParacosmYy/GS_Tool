#include "a9980/m9980.h"
QVector<double> m9980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
