#include "i25248/m25248.h"
QVector<double> m25248::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
