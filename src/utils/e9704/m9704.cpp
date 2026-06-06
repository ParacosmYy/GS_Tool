#include "e9704/m9704.h"
QVector<double> m9704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
