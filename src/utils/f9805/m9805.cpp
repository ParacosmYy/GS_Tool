#include "f9805/m9805.h"
QVector<double> m9805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
