#include "k29590/m29590.h"
QVector<double> m29590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
