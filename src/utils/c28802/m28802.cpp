#include "c28802/m28802.h"
QVector<double> m28802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
