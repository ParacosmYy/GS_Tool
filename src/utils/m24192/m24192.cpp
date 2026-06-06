#include "m24192/m24192.h"
QVector<double> m24192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
