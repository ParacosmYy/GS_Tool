#include "b28061/m28061.h"
QVector<double> m28061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
