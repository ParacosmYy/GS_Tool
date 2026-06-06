#include "g8406/m8406.h"
QVector<double> m8406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
