#include "l24051/m24051.h"
QVector<double> m24051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
