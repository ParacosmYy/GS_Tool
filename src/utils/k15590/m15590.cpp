#include "k15590/m15590.h"
QVector<double> m15590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
