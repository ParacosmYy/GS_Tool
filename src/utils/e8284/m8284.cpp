#include "e8284/m8284.h"
QVector<double> m8284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
