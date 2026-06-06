#include "e8044/m8044.h"
QVector<double> m8044::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
