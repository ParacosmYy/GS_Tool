#include "k9590/m9590.h"
QVector<double> m9590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
