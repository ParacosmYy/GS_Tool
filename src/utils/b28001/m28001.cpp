#include "b28001/m28001.h"
QVector<double> m28001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
