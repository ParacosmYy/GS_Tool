#include "k33150/m33150.h"
QVector<double> m33150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
