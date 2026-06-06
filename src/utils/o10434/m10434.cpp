#include "o10434/m10434.h"
QVector<double> m10434::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
