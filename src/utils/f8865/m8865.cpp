#include "f8865/m8865.h"
QVector<double> m8865::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
