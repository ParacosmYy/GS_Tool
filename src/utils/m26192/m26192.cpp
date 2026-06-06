#include "m26192/m26192.h"
QVector<double> m26192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
