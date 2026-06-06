#include "a8260/m8260.h"
QVector<double> m8260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
