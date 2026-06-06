#include "a18700/m18700.h"
QVector<double> m18700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
