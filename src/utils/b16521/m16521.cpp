#include "b16521/m16521.h"
QVector<double> m16521::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
