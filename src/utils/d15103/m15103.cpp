#include "d15103/m15103.h"
QVector<double> m15103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
