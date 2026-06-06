#include "a8700/m8700.h"
QVector<double> m8700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
