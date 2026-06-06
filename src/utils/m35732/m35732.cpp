#include "m35732/m35732.h"
QVector<double> m35732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
