#include "j7849/m7849.h"
QVector<double> m7849::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
