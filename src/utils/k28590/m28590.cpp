#include "k28590/m28590.h"
QVector<double> m28590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
