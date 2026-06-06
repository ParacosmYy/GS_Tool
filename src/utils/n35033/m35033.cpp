#include "n35033/m35033.h"
QVector<double> m35033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
