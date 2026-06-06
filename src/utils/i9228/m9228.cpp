#include "i9228/m9228.h"
QVector<double> m9228::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
