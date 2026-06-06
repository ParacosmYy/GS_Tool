#include "b19121/m19121.h"
QVector<double> m19121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
