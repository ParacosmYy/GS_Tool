#include "a9700/m9700.h"
QVector<double> m9700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
