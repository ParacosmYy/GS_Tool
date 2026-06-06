#include "b15921/m15921.h"
QVector<double> m15921::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
