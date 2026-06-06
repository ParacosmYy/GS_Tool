#include "b25401/m25401.h"
QVector<double> m25401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
