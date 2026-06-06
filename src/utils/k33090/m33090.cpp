#include "k33090/m33090.h"
QVector<double> m33090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
