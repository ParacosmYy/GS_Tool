#include "m7852/m7852.h"
QVector<double> m7852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
