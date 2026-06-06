#include "k16270/m16270.h"
QVector<double> m16270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
