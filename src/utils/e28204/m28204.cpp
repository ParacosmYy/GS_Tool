#include "e28204/m28204.h"
QVector<double> m28204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
