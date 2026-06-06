#include "l15051/m15051.h"
QVector<double> m15051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
