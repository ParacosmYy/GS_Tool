#include "m28792/m28792.h"
QVector<double> m28792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
