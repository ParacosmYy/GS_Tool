#include "a35400/m35400.h"
QVector<double> m35400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
