#include "p18015/m18015.h"
QVector<double> m18015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
