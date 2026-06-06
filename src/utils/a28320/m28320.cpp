#include "a28320/m28320.h"
QVector<double> m28320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
