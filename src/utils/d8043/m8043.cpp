#include "d8043/m8043.h"
QVector<double> m8043::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
