#include "e26004/m26004.h"
QVector<double> m26004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
