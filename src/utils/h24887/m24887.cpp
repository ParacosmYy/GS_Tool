#include "h24887/m24887.h"
QVector<double> m24887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
