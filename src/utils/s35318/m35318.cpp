#include "s35318/m35318.h"
QVector<double> m35318::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
