#include "a9400/m9400.h"
QVector<double> m9400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
