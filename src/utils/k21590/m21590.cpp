#include "k21590/m21590.h"
QVector<double> m21590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
