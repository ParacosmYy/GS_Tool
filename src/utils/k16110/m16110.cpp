#include "k16110/m16110.h"
QVector<double> m16110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
