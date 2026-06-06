#include "k27550/m27550.h"
QVector<double> m27550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
