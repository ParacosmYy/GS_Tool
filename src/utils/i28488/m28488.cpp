#include "i28488/m28488.h"
QVector<double> m28488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
