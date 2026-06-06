#include "s8858/m8858.h"
QVector<double> m8858::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
