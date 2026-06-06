#include "d18823/m18823.h"
QVector<double> m18823::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
