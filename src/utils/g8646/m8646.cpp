#include "g8646/m8646.h"
QVector<double> m8646::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
