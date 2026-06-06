#include "a18400/m18400.h"
QVector<double> m18400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
