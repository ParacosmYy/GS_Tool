#include "i8048/m8048.h"
QVector<double> m8048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
