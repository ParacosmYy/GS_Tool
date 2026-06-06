#include "a14400/m14400.h"
QVector<double> m14400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
