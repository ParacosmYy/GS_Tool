#include "a28260/m28260.h"
QVector<double> m28260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
