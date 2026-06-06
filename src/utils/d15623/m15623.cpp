#include "d15623/m15623.h"
QVector<double> m15623::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
