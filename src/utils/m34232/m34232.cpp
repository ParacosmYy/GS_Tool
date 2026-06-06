#include "m34232/m34232.h"
QVector<double> m34232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
