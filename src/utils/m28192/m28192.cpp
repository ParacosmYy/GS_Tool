#include "m28192/m28192.h"
QVector<double> m28192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
