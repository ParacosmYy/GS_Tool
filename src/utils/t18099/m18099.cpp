#include "t18099/m18099.h"
QVector<double> m18099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
