#include "n28013/m28013.h"
QVector<double> m28013::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
