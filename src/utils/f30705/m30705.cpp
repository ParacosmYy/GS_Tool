#include "f30705/m30705.h"
QVector<double> m30705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
