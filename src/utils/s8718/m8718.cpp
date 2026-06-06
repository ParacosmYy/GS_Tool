#include "s8718/m8718.h"
QVector<double> m8718::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
