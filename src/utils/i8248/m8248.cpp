#include "i8248/m8248.h"
QVector<double> m8248::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
