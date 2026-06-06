#include "p18715/m18715.h"
QVector<double> m18715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
