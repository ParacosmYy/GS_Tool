#include "k31590/m31590.h"
QVector<double> m31590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
