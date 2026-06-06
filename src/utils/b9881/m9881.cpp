#include "b9881/m9881.h"
QVector<double> m9881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
