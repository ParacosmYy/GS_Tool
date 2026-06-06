#include "a9880/m9880.h"
QVector<double> m9880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
