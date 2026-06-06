#include "g35786/m35786.h"
QVector<double> m35786::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
