#include "i9968/m9968.h"
QVector<double> m9968::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
