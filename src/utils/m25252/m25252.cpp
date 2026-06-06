#include "m25252/m25252.h"
QVector<double> m25252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
