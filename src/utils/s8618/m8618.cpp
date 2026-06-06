#include "s8618/m8618.h"
QVector<double> m8618::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
