#include "t16559/m16559.h"
QVector<double> m16559::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
