#include "b16661/m16661.h"
QVector<double> m16661::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
