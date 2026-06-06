#include "k21850/m21850.h"
QVector<double> m21850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
