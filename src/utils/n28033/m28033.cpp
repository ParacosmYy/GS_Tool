#include "n28033/m28033.h"
QVector<double> m28033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
