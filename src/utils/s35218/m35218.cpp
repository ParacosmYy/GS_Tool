#include "s35218/m35218.h"
QVector<double> m35218::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
