#include "f9865/m9865.h"
QVector<double> m9865::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
