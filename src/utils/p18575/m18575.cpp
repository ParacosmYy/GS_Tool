#include "p18575/m18575.h"
QVector<double> m18575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
