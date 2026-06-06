#include "f9125/m9125.h"
QVector<double> m9125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
