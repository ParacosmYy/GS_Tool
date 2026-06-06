#include "a32400/m32400.h"
QVector<double> m32400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
