#include "i28848/m28848.h"
QVector<double> m28848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
