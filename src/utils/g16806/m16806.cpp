#include "g16806/m16806.h"
QVector<double> m16806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
