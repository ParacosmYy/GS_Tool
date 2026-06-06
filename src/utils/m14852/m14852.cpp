#include "m14852/m14852.h"
QVector<double> m14852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
