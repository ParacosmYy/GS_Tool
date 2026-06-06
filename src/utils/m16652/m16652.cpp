#include "m16652/m16652.h"
QVector<double> m16652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
