#include "e24004/m24004.h"
QVector<double> m24004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
