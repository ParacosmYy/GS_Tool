#include "i35808/m35808.h"
QVector<double> m35808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
