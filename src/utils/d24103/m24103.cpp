#include "d24103/m24103.h"
QVector<double> m24103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
