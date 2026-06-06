#include "t35099/m35099.h"
QVector<double> m35099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
