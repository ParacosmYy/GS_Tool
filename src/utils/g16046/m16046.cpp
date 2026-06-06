#include "g16046/m16046.h"
QVector<double> m16046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
