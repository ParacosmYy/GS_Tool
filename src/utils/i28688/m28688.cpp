#include "i28688/m28688.h"
QVector<double> m28688::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
