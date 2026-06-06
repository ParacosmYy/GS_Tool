#include "o16354/m16354.h"
QVector<double> m16354::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
