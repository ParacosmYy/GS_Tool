#include "k37030/m37030.h"
QVector<double> m37030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
