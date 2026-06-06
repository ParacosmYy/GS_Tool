#include "k15110/m15110.h"
QVector<double> m15110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
