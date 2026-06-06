#include "n15253/m15253.h"
QVector<double> m15253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
