#include "k30590/m30590.h"
QVector<double> m30590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
