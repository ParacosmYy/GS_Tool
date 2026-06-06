#include "f19805/m19805.h"
QVector<double> m19805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
