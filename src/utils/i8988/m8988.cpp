#include "i8988/m8988.h"
QVector<double> m8988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
