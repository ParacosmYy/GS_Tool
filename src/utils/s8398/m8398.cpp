#include "s8398/m8398.h"
QVector<double> m8398::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
