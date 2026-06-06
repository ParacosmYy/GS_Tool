#include "m8992/m8992.h"
QVector<double> m8992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
