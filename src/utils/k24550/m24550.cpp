#include "k24550/m24550.h"
QVector<double> m24550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
