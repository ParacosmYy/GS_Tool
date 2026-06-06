#include "d7983/m7983.h"
QVector<double> m7983::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
