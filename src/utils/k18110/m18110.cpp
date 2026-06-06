#include "k18110/m18110.h"
QVector<double> m18110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
