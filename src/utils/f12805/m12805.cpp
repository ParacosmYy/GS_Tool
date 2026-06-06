#include "f12805/m12805.h"
QVector<double> m12805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
