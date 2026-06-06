#include "m9672/m9672.h"
QVector<double> m9672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
