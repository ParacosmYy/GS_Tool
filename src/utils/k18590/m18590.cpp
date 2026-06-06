#include "k18590/m18590.h"
QVector<double> m18590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
