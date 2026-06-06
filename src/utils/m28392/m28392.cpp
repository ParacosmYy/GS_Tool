#include "m28392/m28392.h"
QVector<double> m28392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
