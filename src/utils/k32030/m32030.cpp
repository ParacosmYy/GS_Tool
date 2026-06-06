#include "k32030/m32030.h"
QVector<double> m32030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
