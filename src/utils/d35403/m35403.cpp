#include "d35403/m35403.h"
QVector<double> m35403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
