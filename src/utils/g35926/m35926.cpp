#include "g35926/m35926.h"
QVector<double> m35926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
