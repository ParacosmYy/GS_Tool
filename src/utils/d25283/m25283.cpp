#include "d25283/m25283.h"
QVector<double> m25283::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
