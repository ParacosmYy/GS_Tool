#include "l8911/m8911.h"
QVector<double> m8911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
