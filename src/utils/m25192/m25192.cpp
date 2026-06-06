#include "m25192/m25192.h"
QVector<double> m25192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
