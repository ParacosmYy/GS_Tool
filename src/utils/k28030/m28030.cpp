#include "k28030/m28030.h"
QVector<double> m28030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
