#include "a35600/m35600.h"
QVector<double> m35600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
