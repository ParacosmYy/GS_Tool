#include "a28580/m28580.h"
QVector<double> m28580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
