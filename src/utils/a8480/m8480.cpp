#include "a8480/m8480.h"
QVector<double> m8480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
