#include "s25618/m25618.h"
QVector<double> m25618::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
