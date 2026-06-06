#include "s8758/m8758.h"
QVector<double> m8758::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
