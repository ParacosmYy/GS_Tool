#include "m26812/m26812.h"
QVector<double> m26812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
