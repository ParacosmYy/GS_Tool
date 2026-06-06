#include "l37711/m37711.h"
QVector<double> m37711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
