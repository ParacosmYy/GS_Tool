#include "a9480/m9480.h"
QVector<double> m9480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
