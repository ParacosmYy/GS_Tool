#include "k25390/m25390.h"
QVector<double> m25390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
