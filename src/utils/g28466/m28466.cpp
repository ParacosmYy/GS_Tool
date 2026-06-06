#include "g28466/m28466.h"
QVector<double> m28466::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
