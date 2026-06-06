#include "k16550/m16550.h"
QVector<double> m16550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
