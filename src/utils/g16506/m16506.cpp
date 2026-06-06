#include "g16506/m16506.h"
QVector<double> m16506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
