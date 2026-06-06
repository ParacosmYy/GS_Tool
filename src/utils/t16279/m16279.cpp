#include "t16279/m16279.h"
QVector<double> m16279::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
