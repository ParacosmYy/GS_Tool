#include "s16618/m16618.h"
QVector<double> m16618::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
