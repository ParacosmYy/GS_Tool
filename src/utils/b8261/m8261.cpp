#include "b8261/m8261.h"
QVector<double> m8261::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
