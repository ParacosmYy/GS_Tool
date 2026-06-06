#include "c35502/m35502.h"
QVector<double> m35502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
