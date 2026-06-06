#include "o9054/m9054.h"
QVector<double> m9054::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
