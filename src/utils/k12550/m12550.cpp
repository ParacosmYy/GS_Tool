#include "k12550/m12550.h"
QVector<double> m12550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
