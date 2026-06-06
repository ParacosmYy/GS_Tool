#include "f8645/m8645.h"
QVector<double> m8645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
