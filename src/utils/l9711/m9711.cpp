#include "l9711/m9711.h"
QVector<double> m9711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
