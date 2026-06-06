#include "d25083/m25083.h"
QVector<double> m25083::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
