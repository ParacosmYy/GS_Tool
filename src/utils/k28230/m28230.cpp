#include "k28230/m28230.h"
QVector<double> m28230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
