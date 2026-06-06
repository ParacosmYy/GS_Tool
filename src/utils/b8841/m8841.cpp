#include "b8841/m8841.h"
QVector<double> m8841::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
