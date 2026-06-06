#include "m8732/m8732.h"
QVector<double> m8732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
