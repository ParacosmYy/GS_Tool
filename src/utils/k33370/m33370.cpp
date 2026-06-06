#include "k33370/m33370.h"
QVector<double> m33370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
