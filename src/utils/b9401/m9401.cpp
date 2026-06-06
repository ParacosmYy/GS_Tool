#include "b9401/m9401.h"
QVector<double> m9401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
