#include "t16099/m16099.h"
QVector<double> m16099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
