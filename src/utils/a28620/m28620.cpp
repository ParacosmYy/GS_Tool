#include "a28620/m28620.h"
QVector<double> m28620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
