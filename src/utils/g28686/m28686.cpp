#include "g28686/m28686.h"
QVector<double> m28686::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
