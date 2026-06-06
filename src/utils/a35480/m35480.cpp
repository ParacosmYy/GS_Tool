#include "a35480/m35480.h"
QVector<double> m35480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
