#include "a35200/m35200.h"
QVector<double> m35200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
