#include "g16226/m16226.h"
QVector<double> m16226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
