#include "a19880/m19880.h"
QVector<double> m19880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
