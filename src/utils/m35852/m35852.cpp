#include "m35852/m35852.h"
QVector<double> m35852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
