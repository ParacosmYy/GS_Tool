#include "b9981/m9981.h"
QVector<double> m9981::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
