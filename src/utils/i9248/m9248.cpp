#include "i9248/m9248.h"
QVector<double> m9248::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
