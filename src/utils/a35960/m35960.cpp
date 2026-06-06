#include "a35960/m35960.h"
QVector<double> m35960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
