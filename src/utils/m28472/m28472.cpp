#include "m28472/m28472.h"
QVector<double> m28472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
