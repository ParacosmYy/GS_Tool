#include "k28330/m28330.h"
QVector<double> m28330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
