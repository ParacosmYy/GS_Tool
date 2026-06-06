#include "s25318/m25318.h"
QVector<double> m25318::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
