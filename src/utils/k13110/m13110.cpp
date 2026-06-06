#include "k13110/m13110.h"
QVector<double> m13110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
