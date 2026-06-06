#include "a36400/m36400.h"
QVector<double> m36400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
