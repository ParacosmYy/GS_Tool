#include "k32590/m32590.h"
QVector<double> m32590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
