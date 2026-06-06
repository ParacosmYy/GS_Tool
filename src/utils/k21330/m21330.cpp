#include "k21330/m21330.h"
QVector<double> m21330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
