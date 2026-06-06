#include "m28132/m28132.h"
QVector<double> m28132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
