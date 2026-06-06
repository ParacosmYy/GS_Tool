#include "o32474/m32474.h"
QVector<double> m32474::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
