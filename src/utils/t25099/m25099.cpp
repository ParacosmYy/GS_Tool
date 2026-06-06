#include "t25099/m25099.h"
QVector<double> m25099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
