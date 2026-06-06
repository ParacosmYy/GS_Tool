#include "k28850/m28850.h"
QVector<double> m28850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
