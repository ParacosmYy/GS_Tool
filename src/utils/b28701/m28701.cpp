#include "b28701/m28701.h"
QVector<double> m28701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
