#include "k9610/m9610.h"
QVector<double> m9610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
