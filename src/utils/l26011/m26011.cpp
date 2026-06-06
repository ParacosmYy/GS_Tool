#include "l26011/m26011.h"
QVector<double> m26011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
