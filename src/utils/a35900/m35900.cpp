#include "a35900/m35900.h"
QVector<double> m35900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
