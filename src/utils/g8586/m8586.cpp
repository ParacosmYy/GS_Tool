#include "g8586/m8586.h"
QVector<double> m8586::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
