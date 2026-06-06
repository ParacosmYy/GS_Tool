#include "d9043/m9043.h"
QVector<double> m9043::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
