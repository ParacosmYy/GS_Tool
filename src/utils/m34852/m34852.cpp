#include "m34852/m34852.h"
QVector<double> m34852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
