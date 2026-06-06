#include "d25623/m25623.h"
QVector<double> m25623::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
