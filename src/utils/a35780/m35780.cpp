#include "a35780/m35780.h"
QVector<double> m35780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
