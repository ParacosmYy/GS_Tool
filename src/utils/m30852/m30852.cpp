#include "m30852/m30852.h"
QVector<double> m30852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
