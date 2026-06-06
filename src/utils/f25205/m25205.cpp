#include "f25205/m25205.h"
QVector<double> m25205::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
