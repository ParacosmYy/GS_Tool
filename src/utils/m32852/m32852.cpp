#include "m32852/m32852.h"
QVector<double> m32852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
