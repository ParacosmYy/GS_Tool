#include "k21150/m21150.h"
QVector<double> m21150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
