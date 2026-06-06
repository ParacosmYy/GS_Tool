#include "i28868/m28868.h"
QVector<double> m28868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
