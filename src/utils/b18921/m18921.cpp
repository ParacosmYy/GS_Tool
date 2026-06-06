#include "b18921/m18921.h"
QVector<double> m18921::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
