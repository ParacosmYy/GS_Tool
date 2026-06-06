#include "b9921/m9921.h"
QVector<double> m9921::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
