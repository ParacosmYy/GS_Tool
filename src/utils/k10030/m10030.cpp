#include "k10030/m10030.h"
QVector<double> m10030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
