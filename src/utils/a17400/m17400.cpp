#include "a17400/m17400.h"
QVector<double> m17400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
