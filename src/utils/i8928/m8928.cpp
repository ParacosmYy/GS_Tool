#include "i8928/m8928.h"
QVector<double> m8928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
