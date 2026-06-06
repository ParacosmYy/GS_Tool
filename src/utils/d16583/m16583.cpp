#include "d16583/m16583.h"
QVector<double> m16583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
