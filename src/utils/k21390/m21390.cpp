#include "k21390/m21390.h"
QVector<double> m21390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
