#include "a12600/m12600.h"
QVector<double> m12600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
