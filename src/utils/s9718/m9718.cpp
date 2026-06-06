#include "s9718/m9718.h"
QVector<double> m9718::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
