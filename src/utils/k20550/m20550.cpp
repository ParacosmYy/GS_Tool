#include "k20550/m20550.h"
QVector<double> m20550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
