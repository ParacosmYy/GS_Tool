#include "f9925/m9925.h"
QVector<double> m9925::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
