#include "k28530/m28530.h"
QVector<double> m28530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
