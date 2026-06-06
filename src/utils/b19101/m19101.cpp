#include "b19101/m19101.h"
QVector<double> m19101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
