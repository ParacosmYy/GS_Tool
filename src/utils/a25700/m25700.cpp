#include "a25700/m25700.h"
QVector<double> m25700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
