#include "k16090/m16090.h"
QVector<double> m16090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
