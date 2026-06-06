#include "k28010/m28010.h"
QVector<double> m28010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
