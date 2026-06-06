#include "g9926/m9926.h"
QVector<double> m9926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
