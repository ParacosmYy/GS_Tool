#include "b19881/m19881.h"
QVector<double> m19881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
