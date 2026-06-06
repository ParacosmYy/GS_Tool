#include "b19001/m19001.h"
QVector<double> m19001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
