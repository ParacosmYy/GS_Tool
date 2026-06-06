#include "o9354/m9354.h"
QVector<double> m9354::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
