#include "k21550/m21550.h"
QVector<double> m21550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
