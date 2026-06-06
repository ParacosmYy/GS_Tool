#include "o24014/m24014.h"
QVector<double> m24014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
