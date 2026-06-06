#include "m20192/m20192.h"
QVector<double> m20192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
