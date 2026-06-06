#include "h28127/m28127.h"
QVector<double> m28127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
