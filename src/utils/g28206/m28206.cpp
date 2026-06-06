#include "g28206/m28206.h"
QVector<double> m28206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
