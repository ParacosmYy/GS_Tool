#include "h19607/m19607.h"
QVector<double> m19607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
